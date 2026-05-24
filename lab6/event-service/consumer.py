import json
import os
import signal
import sys
import time
from typing import Any

import pika


RABBITMQ_HOST = os.getenv("RABBITMQ_HOST", "rabbitmq")
RABBITMQ_PORT = int(os.getenv("RABBITMQ_PORT", "5672"))
RABBITMQ_USER = os.getenv("RABBITMQ_USER", "guest")
RABBITMQ_PASSWORD = os.getenv("RABBITMQ_PASSWORD", "guest")

EXCHANGE = os.getenv("EVENT_EXCHANGE", "hotel.events")
QUEUE = os.getenv("NOTIFICATIONS_QUEUE", "hotel.notifications")

processed_event_ids: set[str] = set()
should_stop = False


def stop_handler(signum: int, frame: Any) -> None:
    global should_stop
    should_stop = True


def connect_with_retry() -> pika.BlockingConnection:
    credentials = pika.PlainCredentials(RABBITMQ_USER, RABBITMQ_PASSWORD)
    params = pika.ConnectionParameters(
        host=RABBITMQ_HOST,
        port=RABBITMQ_PORT,
        credentials=credentials,
        heartbeat=30,
        blocked_connection_timeout=30,
    )

    for attempt in range(1, 31):
        try:
            return pika.BlockingConnection(params)
        except pika.exceptions.AMQPConnectionError:
            print(f"RabbitMQ is not ready, retry {attempt}/30")
            time.sleep(2)

    raise RuntimeError("RabbitMQ connection failed")


def declare_topology(channel: pika.adapters.blocking_connection.BlockingChannel) -> None:
    channel.exchange_declare(
        exchange=EXCHANGE,
        exchange_type="topic",
        durable=True,
    )

    channel.queue_declare(queue=QUEUE, durable=True)

    for routing_key in (
        "booking.created",
        "booking.cancelled",
        "user.registered",
        "review.created",
    ):
        channel.queue_bind(
            exchange=EXCHANGE,
            queue=QUEUE,
            routing_key=routing_key,
        )


def handle_event(event: dict[str, Any]) -> None:
    event_type = event.get("eventType")
    payload = event.get("payload", {})

    if event_type == "BookingCreated":
        print(
            "Notification prepared: booking "
            f"{payload.get('bookingId')} created for user {payload.get('userId')}"
        )
        return

    if event_type == "BookingCancelled":
        print(
            "Notification prepared: booking "
            f"{payload.get('bookingId')} cancelled for user {payload.get('userId')}"
        )
        return

    if event_type == "UserRegistered":
        print(f"Notification prepared: welcome user {payload.get('userId')}")
        return

    if event_type == "ReviewCreated":
        print(f"Notification prepared: new review for hotel {payload.get('hotelId')}")
        return

    print(f"Unknown event type: {event_type}")


def on_message(channel, method, properties, body: bytes) -> None:
    try:
        event = json.loads(body.decode("utf-8"))
        event_id = event.get("eventId")

        if not event_id:
            raise ValueError("Missing eventId")

        if event_id in processed_event_ids:
            print(f"Duplicate event skipped: {event_id}")
            channel.basic_ack(delivery_tag=method.delivery_tag)
            return

        print(f"Received {event.get('eventType')} event_id={event_id}")
        handle_event(event)

        processed_event_ids.add(event_id)
        channel.basic_ack(delivery_tag=method.delivery_tag)

    except Exception as exc:
        print(f"Failed to process message: {exc}")
        channel.basic_nack(delivery_tag=method.delivery_tag, requeue=False)


def main() -> int:
    signal.signal(signal.SIGTERM, stop_handler)
    signal.signal(signal.SIGINT, stop_handler)

    connection = connect_with_retry()
    channel = connection.channel()

    declare_topology(channel)

    channel.basic_qos(prefetch_count=10)
    channel.basic_consume(
        queue=QUEUE,
        on_message_callback=on_message,
        auto_ack=False,
    )

    print(f"Notification consumer started. queue={QUEUE}")

    while not should_stop:
        connection.process_data_events(time_limit=1)

    print("Consumer stopped")
    connection.close()
    return 0


if __name__ == "__main__":
    sys.exit(main())
