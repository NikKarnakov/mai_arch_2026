# Event Catalog

## Общая информация

Этот каталог описывает события системы бронирования отелей.

В проекте используется RabbitMQ.

```text
exchange: hotel.events
exchange type: topic
message format: JSON
delivery guarantee: at-least-once
```

Основная очередь для реализованного consumer'а:

```text
hotel.notifications
```

Основные реализованные события:

```text
BookingCreated
BookingCancelled
```

Дополнительно проектно описаны события:

```text
UserRegistered
HotelCreated
ReviewCreated
RateLimitExceeded
CacheInvalidated
```

## Общий формат события

Все события публикуются в едином envelope:

```json
{
  "eventId": "evt-2026-000001",
  "eventType": "BookingCreated",
  "eventVersion": 1,
  "occurredAt": "2026-05-24T12:00:00Z",
  "producer": "hotel-booking-api",
  "correlationId": "req-abc-123",
  "payload": {}
}
```

## Поля envelope

| Поле | Тип | Обязательное | Описание |
|---|---|---:|---|
| `eventId` | string | да | уникальный идентификатор события |
| `eventType` | string | да | название события |
| `eventVersion` | number | да | версия схемы события |
| `occurredAt` | string | да | дата и время возникновения события |
| `producer` | string | да | компонент-производитель события |
| `correlationId` | string | нет | идентификатор запроса для трассировки |
| `payload` | object | да | бизнес-данные события |

## Гарантии доставки

Для всех событий используется гарантия:

```text
at-least-once
```

Это значит, что сообщение будет доставлено consumer'у один или несколько раз.

Следствие:

```text
consumer должен быть идемпотентным
дубликаты определяются по eventId
```

В текущей реализации `event-service/consumer.py` хранит обработанные `eventId` в памяти процесса и пропускает повторную обработку одинакового события.

---

# 1. BookingCreated

## Название события

```text
BookingCreated
```

## Назначение

Событие означает, что пользователь успешно создал бронирование.

## Routing

```text
exchange: hotel.events
routing key: booking.created
queue: hotel.notifications
```

## Producer

```text
Booking API / Event Producer
```

В текущей реализации тестовое событие публикуется через:

```text
event-service/producer.py
```

В production-варианте событие должно публиковаться из `BookingsHandler` после успешного `POST /bookings`.

## Consumers

```text
Notification Consumer
Analytics Consumer
Read Model Consumer
Inventory Consumer
```

В текущей реализации событие обрабатывает:

```text
event-service/consumer.py
```

## Payload

```json
{
  "bookingId": 501,
  "userId": 101,
  "roomId": 7,
  "checkIn": "2026-06-10",
  "checkOut": "2026-06-12",
  "guestsCount": 2,
  "status": "created"
}
```

## Payload fields

| Поле | Тип | Обязательное | Описание |
|---|---|---:|---|
| `bookingId` | number | да | идентификатор бронирования |
| `userId` | number | да | идентификатор пользователя |
| `roomId` | number | да | идентификатор номера |
| `checkIn` | string/date | да | дата заезда |
| `checkOut` | string/date | да | дата выезда |
| `guestsCount` | number | да | количество гостей |
| `status` | string | да | статус бронирования |

## Example

```json
{
  "eventId": "evt-booking-501-created",
  "eventType": "BookingCreated",
  "eventVersion": 1,
  "occurredAt": "2026-05-24T12:00:00Z",
  "producer": "hotel-booking-event-producer",
  "correlationId": "req-001",
  "payload": {
    "bookingId": 501,
    "userId": 101,
    "roomId": 7,
    "checkIn": "2026-06-10",
    "checkOut": "2026-06-12",
    "guestsCount": 2,
    "status": "created"
  }
}
```

## Delivery guarantee

```text
at-least-once
```

## Idempotency key

```text
eventId
```

---

# 2. BookingCancelled

## Название события

```text
BookingCancelled
```

## Назначение

Событие означает, что пользователь успешно отменил бронирование.

## Routing

```text
exchange: hotel.events
routing key: booking.cancelled
queue: hotel.notifications
```

## Producer

```text
Booking API / Event Producer
```

В текущей реализации тестовое событие публикуется через:

```text
event-service/producer.py
```

В production-варианте событие должно публиковаться из `BookingsHandler` после успешного `DELETE /bookings`.

## Consumers

```text
Notification Consumer
Analytics Consumer
Read Model Consumer
Inventory Consumer
```

В текущей реализации событие обрабатывает:

```text
event-service/consumer.py
```

## Payload

```json
{
  "bookingId": 501,
  "userId": 101,
  "status": "cancelled"
}
```

## Payload fields

| Поле | Тип | Обязательное | Описание |
|---|---|---:|---|
| `bookingId` | number | да | идентификатор бронирования |
| `userId` | number | да | идентификатор пользователя |
| `status` | string | да | новый статус бронирования |

## Example

```json
{
  "eventId": "evt-booking-501-cancelled",
  "eventType": "BookingCancelled",
  "eventVersion": 1,
  "occurredAt": "2026-05-24T13:00:00Z",
  "producer": "hotel-booking-event-producer",
  "correlationId": "req-002",
  "payload": {
    "bookingId": 501,
    "userId": 101,
    "status": "cancelled"
  }
}
```

## Delivery guarantee

```text
at-least-once
```

## Idempotency key

```text
eventId
```

---

# 3. UserRegistered

## Название события

```text
UserRegistered
```

## Назначение

Событие означает, что новый пользователь успешно зарегистрирован.

## Routing

```text
exchange: hotel.events
routing key: user.registered
queue: hotel.notifications
```

## Producer

```text
RegisterHandler / Identity API
```

## Consumers

```text
Notification Consumer
Analytics Consumer
User Profile Consumer
```

## Payload

```json
{
  "userId": 101,
  "login": "ivanp",
  "email": "ivan.petrov@example.com"
}
```

## Payload fields

| Поле | Тип | Обязательное | Описание |
|---|---|---:|---|
| `userId` | number | да | идентификатор пользователя |
| `login` | string | да | логин пользователя |
| `email` | string | да | email пользователя |

## Example

```json
{
  "eventId": "evt-user-101-registered",
  "eventType": "UserRegistered",
  "eventVersion": 1,
  "occurredAt": "2026-05-24T10:00:00Z",
  "producer": "hotel-booking-api",
  "correlationId": "req-user-001",
  "payload": {
    "userId": 101,
    "login": "ivanp",
    "email": "ivan.petrov@example.com"
  }
}
```

## Delivery guarantee

```text
at-least-once
```

## Idempotency key

```text
eventId
```

---

# 4. HotelCreated

## Название события

```text
HotelCreated
```

## Назначение

Событие означает, что в каталоге создан новый отель.

## Routing

```text
exchange: hotel.events
routing key: hotel.created
queue: hotel.analytics, hotel.read_model
```

## Producer

```text
HotelsHandler / Inventory API
```

## Consumers

```text
Analytics Consumer
Read Model Consumer
Cache Consumer
```

## Payload

```json
{
  "hotelId": 10,
  "name": "Moscow Garden",
  "city": "Moscow",
  "stars": 4
}
```

## Payload fields

| Поле | Тип | Обязательное | Описание |
|---|---|---:|---|
| `hotelId` | number | да | идентификатор отеля |
| `name` | string | да | название отеля |
| `city` | string | да | город |
| `stars` | number | да | количество звёзд |

## Example

```json
{
  "eventId": "evt-hotel-10-created",
  "eventType": "HotelCreated",
  "eventVersion": 1,
  "occurredAt": "2026-05-24T11:00:00Z",
  "producer": "hotel-booking-api",
  "correlationId": "req-hotel-001",
  "payload": {
    "hotelId": 10,
    "name": "Moscow Garden",
    "city": "Moscow",
    "stars": 4
  }
}
```

## Delivery guarantee

```text
at-least-once
```

## Idempotency key

```text
eventId
```

---

# 5. ReviewCreated

## Название события

```text
ReviewCreated
```

## Назначение

Событие означает, что пользователь оставил отзыв об отеле.

## Routing

```text
exchange: hotel.events
routing key: review.created
queue: hotel.notifications, hotel.analytics, hotel.read_model
```

## Producer

```text
ReviewsHandler / MongoReviewsHandler
```

## Consumers

```text
Notification Consumer
Analytics Consumer
Rating Consumer
Read Model Consumer
```

## Payload

```json
{
  "reviewId": 701,
  "hotelId": 10,
  "userId": 101,
  "rating": 5
}
```

## Payload fields

| Поле | Тип | Обязательное | Описание |
|---|---|---:|---|
| `reviewId` | number/string | да | идентификатор отзыва |
| `hotelId` | number/string | да | идентификатор отеля |
| `userId` | number/string | да | идентификатор пользователя |
| `rating` | number | да | оценка пользователя |

## Example

```json
{
  "eventId": "evt-review-701-created",
  "eventType": "ReviewCreated",
  "eventVersion": 1,
  "occurredAt": "2026-05-24T16:00:00Z",
  "producer": "hotel-booking-api",
  "correlationId": "req-review-001",
  "payload": {
    "reviewId": 701,
    "hotelId": 10,
    "userId": 101,
    "rating": 5
  }
}
```

## Delivery guarantee

```text
at-least-once
```

## Idempotency key

```text
eventId
```

---

# 6. RateLimitExceeded

## Название события

```text
RateLimitExceeded
```

## Назначение

Событие означает, что запрос был заблокирован rate limiter'ом.

## Routing

```text
exchange: hotel.events
routing key: security.rate_limit_exceeded
queue: hotel.security, hotel.analytics
```

## Producer

```text
RateLimiter / BookingsHandler
```

## Consumers

```text
Security Consumer
Monitoring Consumer
Analytics Consumer
```

## Payload

```json
{
  "endpoint": "POST /bookings",
  "clientKey": "bookings:user:101",
  "limit": 10,
  "remaining": 0,
  "resetAfterSeconds": 12
}
```

## Payload fields

| Поле | Тип | Обязательное | Описание |
|---|---|---:|---|
| `endpoint` | string | да | endpoint, на котором сработал лимит |
| `clientKey` | string | да | ключ клиента в rate limiter |
| `limit` | number | да | установленный лимит |
| `remaining` | number | да | оставшееся количество запросов |
| `resetAfterSeconds` | number | да | время до восстановления лимита |

## Example

```json
{
  "eventId": "evt-rate-limit-001",
  "eventType": "RateLimitExceeded",
  "eventVersion": 1,
  "occurredAt": "2026-05-24T17:00:00Z",
  "producer": "hotel-booking-api",
  "correlationId": "req-rate-limit-001",
  "payload": {
    "endpoint": "POST /bookings",
    "clientKey": "bookings:user:101",
    "limit": 10,
    "remaining": 0,
    "resetAfterSeconds": 12
  }
}
```

## Delivery guarantee

```text
at-least-once
```

## Idempotency key

```text
eventId
```

---

# 7. CacheInvalidated

## Название события

```text
CacheInvalidated
```

## Назначение

Событие означает, что часть кеша была очищена после изменения данных.

## Routing

```text
exchange: hotel.events
routing key: cache.invalidated
queue: hotel.analytics, hotel.read_model
```

## Producer

```text
ResponseCache / API handlers
```

## Consumers

```text
Analytics Consumer
Read Model Consumer
Monitoring Consumer
```

## Payload

```json
{
  "cachePrefix": "rooms:",
  "reason": "BookingCreated",
  "sourceEventId": "evt-booking-501-created"
}
```

## Payload fields

| Поле | Тип | Обязательное | Описание |
|---|---|---:|---|
| `cachePrefix` | string | да | префикс очищенного кеша |
| `reason` | string | да | причина инвалидации |
| `sourceEventId` | string | нет | событие, которое привело к инвалидации |

## Example

```json
{
  "eventId": "evt-cache-001",
  "eventType": "CacheInvalidated",
  "eventVersion": 1,
  "occurredAt": "2026-05-24T17:30:00Z",
  "producer": "hotel-booking-api",
  "correlationId": "req-cache-001",
  "payload": {
    "cachePrefix": "rooms:",
    "reason": "BookingCreated",
    "sourceEventId": "evt-booking-501-created"
  }
}
```

## Delivery guarantee

```text
at-least-once
```

## Idempotency key

```text
eventId
```

---

# Summary table

| Event | Routing key | Producer | Consumers | Delivery |
|---|---|---|---|---|
| `BookingCreated` | `booking.created` | Booking API / Event Producer | Notification, Analytics, Read Model, Inventory | at-least-once |
| `BookingCancelled` | `booking.cancelled` | Booking API / Event Producer | Notification, Analytics, Read Model, Inventory | at-least-once |
| `UserRegistered` | `user.registered` | Identity API | Notification, Analytics, User Profile | at-least-once |
| `HotelCreated` | `hotel.created` | Inventory API | Analytics, Read Model, Cache | at-least-once |
| `ReviewCreated` | `review.created` | Review API | Notification, Analytics, Rating, Read Model | at-least-once |
| `RateLimitExceeded` | `security.rate_limit_exceeded` | RateLimiter / API | Security, Monitoring, Analytics | at-least-once |
| `CacheInvalidated` | `cache.invalidated` | API / Cache component | Analytics, Read Model, Monitoring | at-least-once |
