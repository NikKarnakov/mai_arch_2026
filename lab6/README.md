# Лабораторная работа №6

Проект — backend-сервис бронирования отелей, аналог booking.com.  
В шестой лабораторной работе к проекту добавлена Event-Driven архитектура на RabbitMQ: описаны события, команды, producers/consumers, CQRS, каталог событий, а также реализован простой producer/consumer для проверки взаимодействия через брокер сообщений.

## Результат

Файлы лабораторной работы:

```text
event_driven_design.md      # описание Event-Driven архитектуры
event_catalog.md            # каталог событий
README.md                   # описание проекта и инструкции запуска
docker-compose.yml          # запуск API, PostgreSQL, MongoDB, RabbitMQ и consumer
```

Исходный код producer/consumer:

```text
event-service/producer.py
event-service/consumer.py
event-service/requirements.txt
```

Код основного API и инфраструктура запуска:

```text
src/
configs/
db/
mongo-init/
Dockerfile
CMakeLists.txt
docker-compose.yaml
docker-compose.yml
```

## Краткая архитектура

Основной API остаётся REST-сервисом на userver.  
RabbitMQ используется для асинхронной обработки событий.

```text
Client
  -> Hotel Booking API
      -> PostgreSQL / MongoDB
      -> RabbitMQ exchange hotel.events
          -> queue hotel.notifications
              -> Notification Consumer
```

В рамках практической реализации добавлен отдельный lightweight event-service на Python:

```text
event-service/producer.py   # публикует тестовые события
event-service/consumer.py   # читает события и имитирует уведомления
```

Такой вариант не ломает текущий C++/userver API и при этом показывает полный Event-Driven цикл:

```text
producer -> RabbitMQ -> queue -> consumer -> обработка события
```

## Пункт 1. Анализ событий в системе

В системе выделены команды, которые инициируют события.

Основные commands:

```text
CreateUserCommand
CreateHotelCommand
CreateBookingCommand
CancelBookingCommand
CreateReviewCommand
```

Основные events:

```text
UserRegistered
HotelCreated
BookingCreated
BookingCancelled
ReviewCreated
RateLimitExceeded
CacheInvalidated
```

Для практической реализации выбраны события:

```text
BookingCreated
BookingCancelled
```

Причина: создание и отмена бронирования являются центральными бизнес-сценариями booking-сервиса.

## Пункт 2. Проектирование Event-Driven архитектуры

Определены producers:

```text
RegisterHandler
HotelsHandler
BookingsHandler
ReviewsHandler
RateLimiter / BookingsHandler
```

Определены consumers:

```text
Notification Consumer
Analytics Consumer
Read Model Consumer
Security / Monitoring Consumer
```

Основной реализуемый поток:

```text
POST /bookings
  -> BookingCreated
  -> RabbitMQ
  -> Notification Consumer
```

Поток отмены:

```text
DELETE /bookings
  -> BookingCancelled
  -> RabbitMQ
  -> Notification Consumer
```

Полное описание находится в:

```text
event_driven_design.md
```

## Пункт 3. Проектирование взаимодействия через брокер сообщений

Выбран брокер:

```text
RabbitMQ
```

Причины выбора:

```text
простая настройка через Docker
понятная модель exchange / queue / routing key
удобен для бизнес-событий
есть Management UI
подходит для at-least-once delivery
```

Настройки:

```text
exchange: hotel.events
exchange type: topic
main queue: hotel.notifications
message format: JSON
delivery guarantee: at-least-once
```

Основные routing keys:

```text
booking.created
booking.cancelled
user.registered
hotel.created
review.created
security.rate_limit_exceeded
cache.invalidated
```

## Пункт 4. Применение CQRS

CQRS применим к системе бронирования, потому что read-операции и write-операции имеют разные требования.

Commands:

```text
POST /auth/register
POST /hotels
POST /bookings
DELETE /bookings
POST /reviews
```

Queries:

```text
GET /users
GET /hotels
GET /rooms
GET /bookings
GET /mongo/bookings
GET /mongo/reviews
```

Общий CQRS-поток:

```text
Command
  -> write model
  -> event
  -> consumer
  -> read model / notification / analytics
```

Для бронирований:

```text
CreateBookingCommand
  -> bookings table
  -> BookingCreated
  -> Notification Consumer / Read Model Consumer
```

```text
CancelBookingCommand
  -> bookings.status = cancelled
  -> BookingCancelled
  -> Notification Consumer / Read Model Consumer
```

## Пункт 5. Реализация простого Event-Driven сервиса

Добавлен RabbitMQ в Docker Compose:

```text
rabbitmq:3.13-management
```

Порты:

```text
5672   AMQP
15672  RabbitMQ Management UI
```

Management UI:

```text
http://localhost:15672
login: guest
password: guest
```

Producer:

```text
event-service/producer.py
```

Он публикует события:

```text
BookingCreated
BookingCancelled
```

Consumer:

```text
event-service/consumer.py
```

Он читает очередь:

```text
hotel.notifications
```

и имитирует отправку уведомлений.

## Пункт 6. Каталог событий

Каталог событий находится в:

```text
event_catalog.md
```

В каталоге для каждого события указаны:

```text
название события
структура payload
producer
consumers
routing key
delivery guarantee
idempotency key
```

Описанные события:

```text
BookingCreated
BookingCancelled
UserRegistered
HotelCreated
ReviewCreated
RateLimitExceeded
CacheInvalidated
```

## Запуск проекта

### 1. Запуск всего окружения

```bash
docker compose up --build
```

Будут запущены:

```text
postgres
mongodb
rabbitmq
api
event-consumer
```

### 2. Проверка API

```bash
curl http://localhost:8080/ping
```

### 3. Проверка RabbitMQ Management UI

Открыть в браузере:

```text
http://localhost:15672
```

Логин и пароль:

```text
guest / guest
```

### 4. Отправка тестового события BookingCreated

```bash
docker compose run --rm event-producer
```

### 5. Отправка тестового события BookingCancelled

```bash
docker compose run --rm \
  -e EVENT_TYPE=BookingCancelled \
  -e BOOKING_ID=501 \
  -e USER_ID=101 \
  event-producer
```

### 6. Просмотр обработки события consumer'ом

```bash
docker compose logs -f event-consumer
```

Ожидаемый результат для `BookingCreated`:

```text
Received BookingCreated event_id=...
Notification prepared: booking 501 created for user 101
```

Ожидаемый результат для `BookingCancelled`:

```text
Received BookingCancelled event_id=...
Notification prepared: booking 501 cancelled for user 101
```

## Проверка очереди RabbitMQ

После запуска можно проверить queue в RabbitMQ UI:

```text
Queues and Streams -> hotel.notifications
```

Exchange:

```text
Exchanges -> hotel.events
```

## Итог

В рамках шестой лабораторной работы реализовано:

```text
анализ событий
Event-Driven дизайн
выбор RabbitMQ
topic exchange и routing keys
описание delivery guarantees
CQRS-дизайн
producer
consumer
event catalog
инструкции запуска и проверки
```
