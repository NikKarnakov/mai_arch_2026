import json
import os
import sys
import time
import uuid
from datetime import datetime, timezone

import pika


RABBITMQ_HOST = os.getenv("RABBITMQ_HOST", "rabbitmq")
RABBITMQ_PORT = int(os.getenv("RABBITMQ_PORT", "5672"))
RABBITMQ_USER = os.getenv("RABBITMQ_USER", "guest")
RABBITMQ_PASSWORD = os.getenv("RABBITMQ_PASSWORD", "guest")

EXCHANGE = os.getenv("EVENT_EXCHANGE", "hotel.events")
NOTIFICATIONS_QUEUE = os.getenv("NOTIFICATIONS_QUEUE", "hotel.notifications")


def utc_now() -> str:
    return datetime.now(timezone.utc).isoformat().replace("+00:00", "Z")


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

    channel.queue_declare(queue=NOTIFICATIONS_QUEUE, durable=True)

    for routing_key in (
        "booking.created",
        "booking.cancelled",
        "user.registered",
        "review.created",
    ):
        channel.queue_bind(
            exchange=EXCHANGE,
            queue=NOTIFICATIONS_QUEUE,
            routing_key=routing_key,
        )


def make_booking_created() -> tuple[str, dict]:
    booking_id = int(os.getenv("BOOKING_ID", "501"))
    user_id = int(os.getenv("USER_ID", "101"))
    room_id = int(os.getenv("ROOM_ID", "7"))

    return "booking.created", {
        "eventId": f"evt-{uuid.uuid4()}",
        "eventType": "BookingCreated",
        "eventVersion": 1,
        "occurredAt": utc_now(),
        "producer": "hotel-booking-event-producer",
        "correlationId": f"req-{uuid.uuid4()}",
        "payload": {
            "bookingId": booking_id,
            "userId": user_id,
            "roomId": room_id,
            "checkIn": os.getenv("CHECK_IN", "2026-06-10"),
            "checkOut": os.getenv("CHECK_OUT", "2026-06-12"),
            "guestsCount": int(os.getenv("GUESTS_COUNT", "2")),
            "status": "created",
        },
    }


def make_booking_cancelled() -> tuple[str, dict]:
    booking_id = int(os.getenv("BOOKING_ID", "501"))
    user_id = int(os.getenv("USER_ID", "101"))

    return "booking.cancelled", {
        "eventId": f"evt-{uuid.uuid4()}",
        "eventType": "BookingCancelled",
        "eventVersion": 1,
        "occurredAt": utc_now(),
        "producer": "hotel-booking-event-producer",
        "correlationId": f"req-{uuid.uuid4()}",
        "payload": {
            "bookingId": booking_id,
            "userId": user_id,
            "status": "cancelled",
        },
    }


def build_event() -> tuple[str, dict]:
    event_type = os.getenv("EVENT_TYPE", "BookingCreated")

    if event_type == "BookingCreated":
        return make_booking_created()

    if event_type == "BookingCancelled":
        return make_booking_cancelled()

    raise ValueError(f"Unsupported EVENT_TYPE: {event_type}")


def main() -> int:
    routing_key, event = build_event()

    connection = connect_with_retry()
    channel = connection.channel()
    declare_topology(channel)

    body = json.dumps(event, ensure_ascii=False).encode("utf-8")

    channel.basic_publish(
        exchange=EXCHANGE,
        routing_key=routing_key,
        body=body,
        properties=pika.BasicProperties(
            content_type="application/json",
            delivery_mode=pika.DeliveryMode.Persistent,
            message_id=event["eventId"],
            type=event["eventType"],
            timestamp=int(time.time()),
        ),
        mandatory=True,
    )

    print(f"Published {event['eventType']} with routing_key={routing_key}")
    print(json.dumps(event, ensure_ascii=False, indent=2))

    connection.close()
    return 0


if __name__ == "__main__":
    sys.exit(main())
