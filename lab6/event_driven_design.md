# Event-Driven Архитектура

## 1. Команды системы

Команда — это намерение выполнить действие. Команда приходит в API от пользователя или другого сервиса и может изменить состояние системы.

| Команда | Endpoint | Что делает |
|---|---|---|
| `CreateUserCommand` | `POST /auth/register` | создаёт нового пользователя |
| `LoginUserCommand` | `POST /auth/login` | выполняет авторизацию пользователя |
| `SearchUsersCommand` | `GET /users?query=...` | ищет пользователей по маске имени/фамилии |
| `CreateHotelCommand` | `POST /hotels` | создаёт новый отель |
| `SearchHotelsCommand` | `GET /hotels?city=...` | ищет отели |
| `CreateBookingCommand` | `POST /bookings` | создаёт бронирование |
| `CancelBookingCommand` | `DELETE /bookings` | отменяет бронирование |
| `CreateReviewCommand` | `POST /reviews` или `POST /mongo/reviews` | создаёт отзыв |

Не каждая команда создаёт событие. Read-команды обычно не публикуют события, потому что они не меняют состояние системы.

## 2. События системы

Событие — это факт, который уже произошёл в системе. Событие не просит выполнить действие, а сообщает другим компонентам, что действие успешно завершилось.

Для системы бронирования выделены следующие события:

```text
UserRegistered
HotelCreated
BookingCreated
BookingCancelled
ReviewCreated
CacheInvalidated
RateLimitExceeded
```

## 3. События и команды, которые их инициируют

| Команда | Событие | Когда публикуется |
|---|---|---|
| `CreateUserCommand` | `UserRegistered` | после успешного создания пользователя |
| `CreateHotelCommand` | `HotelCreated` | после успешного создания отеля |
| `CreateBookingCommand` | `BookingCreated` | после успешного создания бронирования |
| `CancelBookingCommand` | `BookingCancelled` | после успешной отмены бронирования |
| `CreateReviewCommand` | `ReviewCreated` | после успешного создания отзыва |
| `CreateHotelCommand` / `CreateBookingCommand` / `CancelBookingCommand` | `CacheInvalidated` | после очистки связанных cache keys |
| Любой endpoint с rate limiting | `RateLimitExceeded` | если запрос заблокирован rate limiter'ом |

## 4. Какие сервисы должны быть уведомлены

## 4.1 Identity Service

Отвечает за пользователей и авторизацию.

Интересующие события:

```text
UserRegistered
RateLimitExceeded
```

Зачем нужны:

```text
UserRegistered      -> отправить welcome-уведомление, создать профиль пользователя
RateLimitExceeded   -> обнаружить подозрительную активность
```

## 4.2 Inventory Service

Отвечает за отели и номера.

Интересующие события:

```text
HotelCreated
BookingCreated
BookingCancelled
CacheInvalidated
```

Зачем нужны:

```text
HotelCreated        -> обновить read model каталога
BookingCreated      -> обновить доступность номера
BookingCancelled    -> вернуть номер в доступность
CacheInvalidated    -> синхронизировать кеши каталога
```

## 4.3 Booking Service

Отвечает за создание и отмену бронирований.

Производит события:

```text
BookingCreated
BookingCancelled
RateLimitExceeded
```

Потребляет события:

```text
HotelCreated
UserRegistered
```

Зачем потребляет:

```text
HotelCreated        -> обновить локальную read model отелей, если используется CQRS
UserRegistered      -> обновить локальную read model пользователей, если нужна быстрая валидация
```

## 4.4 Notification / Mailer Service

Отвечает за уведомления пользователей.

Интересующие события:

```text
UserRegistered
BookingCreated
BookingCancelled
ReviewCreated
```

Зачем нужны:

```text
UserRegistered      -> отправить приветственное письмо
BookingCreated      -> отправить подтверждение бронирования
BookingCancelled    -> отправить уведомление об отмене
ReviewCreated       -> отправить отелю уведомление о новом отзыве
```

## 4.5 Analytics Service

Отвечает за статистику и отчёты.

Интересующие события:

```text
UserRegistered
HotelCreated
BookingCreated
BookingCancelled
ReviewCreated
RateLimitExceeded
```

Зачем нужны:

```text
UserRegistered      -> рост пользовательской базы
HotelCreated        -> рост каталога
BookingCreated      -> конверсия и выручка
BookingCancelled    -> процент отмен
ReviewCreated       -> качество отелей
RateLimitExceeded   -> подозрительная активность и нагрузка
```

## 4.6 Cache / Read Model Updater

Отвечает за обновление read-моделей и кешей.

Интересующие события:

```text
HotelCreated
BookingCreated
BookingCancelled
ReviewCreated
CacheInvalidated
```

Зачем нужны:

```text
HotelCreated        -> перестроить/очистить кеш списка отелей
BookingCreated      -> обновить доступность номеров
BookingCancelled    -> обновить доступность номеров
ReviewCreated       -> обновить рейтинг отеля
CacheInvalidated    -> выполнить централизованную очистку кешей
```

## 5. Подробный список событий

## 5.1 UserRegistered

Событие означает, что пользователь успешно зарегистрирован.

Инициирующая команда:

```text
CreateUserCommand
```

Производитель:

```text
Identity/API service
```

Потребители:

```text
Notification Service
Analytics Service
User Profile Service
```

Пример полезной нагрузки:

```json
{
  "eventId": "evt-001",
  "eventType": "UserRegistered",
  "occurredAt": "2026-05-24T12:00:00Z",
  "payload": {
    "userId": 101,
    "login": "ivanp",
    "email": "ivan.petrov@example.com"
  }
}
```

## 5.2 HotelCreated

Событие означает, что в каталоге появился новый отель.

Инициирующая команда:

```text
CreateHotelCommand
```

Производитель:

```text
Inventory/API service
```

Потребители:

```text
Search/Read Model Service
Cache Service
Analytics Service
```

Пример payload:

```json
{
  "hotelId": 10,
  "name": "Moscow Garden",
  "city": "Moscow",
  "stars": 4
}
```

## 5.3 BookingCreated

Событие означает, что бронирование успешно создано.

Инициирующая команда:

```text
CreateBookingCommand
```

Производитель:

```text
Booking/API service
```

Потребители:

```text
Notification Service
Analytics Service
Inventory Service
Read Model Service
```

Пример payload:

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

## 5.4 BookingCancelled

Событие означает, что пользователь отменил бронирование.

Инициирующая команда:

```text
CancelBookingCommand
```

Производитель:

```text
Booking/API service
```

Потребители:

```text
Notification Service
Analytics Service
Inventory Service
Read Model Service
```

Пример payload:

```json
{
  "bookingId": 501,
  "userId": 101,
  "status": "cancelled",
  "cancelledAt": "2026-05-24T13:30:00Z"
}
```

## 5.5 ReviewCreated

Событие означает, что пользователь оставил отзыв.

Инициирующая команда:

```text
CreateReviewCommand
```

Производитель:

```text
Review/API service
```

Потребители:

```text
Notification Service
Analytics Service
Rating Service
Read Model Service
```

Пример payload:

```json
{
  "reviewId": 701,
  "hotelId": 10,
  "userId": 101,
  "rating": 5
}
```

## 5.6 RateLimitExceeded

Событие означает, что запрос был заблокирован rate limiter'ом.

Инициирующая команда:

```text
CreateBookingCommand
LoginUserCommand
```

Производитель:

```text
API Gateway / API service
```

Потребители:

```text
Security Service
Analytics Service
Monitoring Service
```

Пример payload:

```json
{
  "endpoint": "POST /bookings",
  "clientKey": "bookings:user:101",
  "limit": 10,
  "remaining": 0,
  "resetAfterSeconds": 12
}
```

## 6. Итог по первому пункту

Для системы бронирования отелей главные бизнес-события:

```text
UserRegistered
HotelCreated
BookingCreated
BookingCancelled
ReviewCreated
```

Для инфраструктурной части также выделены события:

```text
CacheInvalidated
RateLimitExceeded
```

В рамках реализации шестой лабораторной работы основной фокус будет на событиях бронирования:

```text
BookingCreated
BookingCancelled
```

Именно они лучше всего демонстрируют Event-Driven подход: бронирование создаётся или отменяется в основном API, а уведомления, аналитика и обновление read-моделей могут выполняться асинхронно через брокер сообщений.


---

# Проектирование Event-Driven архитектуры

## 1. Общая схема компонентов

Для текущего проекта выбирается гибридная архитектура:

```text
REST API остаётся основным способом синхронного взаимодействия с пользователем
RabbitMQ используется для асинхронных бизнес-событий
```

Компоненты системы:

```text
API Service
PostgreSQL
MongoDB
RabbitMQ
Notification Consumer
Analytics Consumer
Read Model / Cache Consumer
```

Логическая схема:

```text
Client
  -> API Service
      -> PostgreSQL / MongoDB
      -> RabbitMQ exchange
          -> notification.booking queue
          -> analytics.booking queue
          -> readmodel.booking queue
```

Архитектурно producer и consumer разделены по ответственности:

```text
producer публикует событие
consumer получает событие и выполняет побочную обработку
```

## 2. Event producers

Event producer — компонент, который публикует событие после успешного изменения состояния системы.

## 2.1 Booking API / BookingsHandler

Производит события:

```text
BookingCreated
BookingCancelled
RateLimitExceeded
```

Когда публикует:

```text
BookingCreated     после успешного POST /bookings
BookingCancelled   после успешного DELETE /bookings
RateLimitExceeded  если POST /bookings заблокирован rate limiter'ом
```

Почему producer именно здесь:

```text
handler знает, что бизнес-операция завершилась успешно
handler получает userId из авторизации
handler получает bookingId после записи в PostgreSQL
```

## 2.2 Hotel API / HotelsHandler

Производит событие:

```text
HotelCreated
```

Когда публикует:

```text
после успешного POST /hotels
```

Зачем нужно событие:

```text
обновить поисковую read model
инвалидировать кеш каталога
записать событие в аналитику
```

В реализации шестой лабораторной основной producer — `BookingsHandler`.

## 2.3 Auth API / RegisterHandler

Производит событие:

```text
UserRegistered
```

Когда публикует:

```text
после успешного POST /auth/register
```

Зачем нужно:

```text
отправить welcome-уведомление
создать профиль пользователя
передать событие в аналитику
```

## 3. Event consumers

Event consumer — компонент, который подписывается на очередь и обрабатывает события.

## 3.1 Notification Consumer

Потребляет события:

```text
UserRegistered
BookingCreated
BookingCancelled
ReviewCreated
```

Что делает:

```text
логирует уведомление
в production-версии отправляет email/SMS/push
```

Пример обработки:

```text
BookingCreated -> отправить пользователю подтверждение бронирования
BookingCancelled -> отправить уведомление об отмене
```

## 3.2 Analytics Consumer

Потребляет события:

```text
UserRegistered
HotelCreated
BookingCreated
BookingCancelled
ReviewCreated
RateLimitExceeded
```

Что делает:

```text
считает конверсию
считает количество бронирований
анализирует отмены
анализирует подозрительную активность
```

## 3.3 Read Model / Cache Consumer

Потребляет события:

```text
HotelCreated
BookingCreated
BookingCancelled
ReviewCreated
```

Что делает:

```text
обновляет read model
инвалидирует кеш
пересчитывает рейтинг отеля
обновляет доступность номеров
```

В текущей реализации кеш уже инвалидируется синхронно в API. Event-Driven вариант позволяет вынести эту логику в отдельный consumer.

## 4. Типы событий

Для проекта выделяются два типа событий:

```text
business events
technical events
```

## 4.1 Business events

Business events описывают изменения предметной области:

```text
UserRegistered
HotelCreated
BookingCreated
BookingCancelled
ReviewCreated
```

Эти события важны для других сервисов и аналитики.

## 4.2 Technical events

Technical events описывают инфраструктурные факты:

```text
CacheInvalidated
RateLimitExceeded
```

Они полезны для мониторинга, безопасности и диагностики.

## 5. Базовая структура сообщения

Все события используют общий envelope:

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

Назначение полей:

| Поле | Описание |
|---|---|
| `eventId` | уникальный идентификатор события |
| `eventType` | тип события |
| `eventVersion` | версия схемы события |
| `occurredAt` | время возникновения события |
| `producer` | сервис-производитель |
| `correlationId` | идентификатор запроса для трассировки |
| `payload` | бизнес-данные события |

## 6. Payload событий

## 6.1 BookingCreated

```json
{
  "eventId": "evt-001",
  "eventType": "BookingCreated",
  "eventVersion": 1,
  "occurredAt": "2026-05-24T12:00:00Z",
  "producer": "hotel-booking-api",
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

Producer:

```text
BookingsHandler
```

Consumers:

```text
Notification Consumer
Analytics Consumer
Read Model Consumer
```

## 6.2 BookingCancelled

```json
{
  "eventId": "evt-002",
  "eventType": "BookingCancelled",
  "eventVersion": 1,
  "occurredAt": "2026-05-24T13:00:00Z",
  "producer": "hotel-booking-api",
  "correlationId": "req-002",
  "payload": {
    "bookingId": 501,
    "userId": 101,
    "status": "cancelled"
  }
}
```

Producer:

```text
BookingsHandler
```

Consumers:

```text
Notification Consumer
Analytics Consumer
Read Model Consumer
```

## 6.3 HotelCreated

```json
{
  "eventId": "evt-003",
  "eventType": "HotelCreated",
  "eventVersion": 1,
  "occurredAt": "2026-05-24T14:00:00Z",
  "producer": "hotel-booking-api",
  "correlationId": "req-003",
  "payload": {
    "hotelId": 10,
    "name": "Moscow Garden",
    "city": "Moscow",
    "stars": 4
  }
}
```

Producer:

```text
HotelsHandler
```

Consumers:

```text
Analytics Consumer
Read Model Consumer
Cache Consumer
```

## 6.4 UserRegistered

```json
{
  "eventId": "evt-004",
  "eventType": "UserRegistered",
  "eventVersion": 1,
  "occurredAt": "2026-05-24T15:00:00Z",
  "producer": "hotel-booking-api",
  "correlationId": "req-004",
  "payload": {
    "userId": 101,
    "login": "ivanp",
    "email": "ivan.petrov@example.com"
  }
}
```

Producer:

```text
RegisterHandler
```

Consumers:

```text
Notification Consumer
Analytics Consumer
```

## 6.5 RateLimitExceeded

```json
{
  "eventId": "evt-005",
  "eventType": "RateLimitExceeded",
  "eventVersion": 1,
  "occurredAt": "2026-05-24T15:30:00Z",
  "producer": "hotel-booking-api",
  "correlationId": "req-005",
  "payload": {
    "endpoint": "POST /bookings",
    "clientKey": "bookings:user:101",
    "limit": 10,
    "remaining": 0,
    "resetAfterSeconds": 12
  }
}
```

Producer:

```text
RateLimiter / BookingsHandler
```

Consumers:

```text
Analytics Consumer
Security Consumer
Monitoring Consumer
```

## 7. Поток событий: создание бронирования

Последовательность:

```text
1. Client отправляет POST /bookings.
2. API проверяет Authorization header.
3. RateLimiter проверяет лимит пользователя.
4. API проверяет доступность номера.
5. API создаёт бронирование в PostgreSQL.
6. API инвалидирует кеш rooms.
7. API публикует BookingCreated в RabbitMQ.
8. Notification Consumer получает событие и формирует уведомление.
9. Analytics Consumer получает событие и обновляет статистику.
10. Read Model Consumer получает событие и обновляет read model доступности.
11. Client получает HTTP 201 Created.
```

Важно:

```text
событие публикуется только после успешной записи бронирования
если запись в БД не удалась, BookingCreated не публикуется
```

## 8. Поток событий: отмена бронирования

Последовательность:

```text
1. Client отправляет DELETE /bookings.
2. API проверяет Authorization header.
3. API отменяет бронирование в PostgreSQL.
4. API инвалидирует кеш rooms.
5. API публикует BookingCancelled в RabbitMQ.
6. Notification Consumer получает событие и формирует уведомление.
7. Analytics Consumer получает событие и фиксирует отмену.
8. Read Model Consumer обновляет доступность номера.
9. Client получает HTTP 200 OK.
```

Если бронирование не найдено или принадлежит другому пользователю:

```text
BookingCancelled не публикуется
```

## 9. Поток событий: превышение rate limit

Последовательность:

```text
1. Client отправляет POST /bookings.
2. API извлекает userId из токена.
3. RateLimiter проверяет Token Bucket.
4. Если токенов нет, API возвращает 429 Too Many Requests.
5. API публикует RateLimitExceeded.
6. Monitoring/Analytics Consumer фиксирует превышение лимита.
```

Это событие помогает обнаруживать:

```text
ошибочные retry клиента
подозрительную активность
попытки перегрузить систему
```

## 10. Итог по второму пункту

В Event-Driven архитектуре проекта:

```text
producer'ы:
- RegisterHandler
- HotelsHandler
- BookingsHandler
- ReviewsHandler
- RateLimiter/BookingsHandler

consumer'ы:
- Notification Consumer
- Analytics Consumer
- Read Model Consumer
- Monitoring/Security Consumer
```

Для реализации шестой лабораторной работы выбирается минимальный рабочий сценарий:

```text
BookingCreated
BookingCancelled
Notification Consumer
RabbitMQ
```

Такой сценарий достаточно простой для запуска, но показывает полный Event-Driven цикл:

```text
команда -> изменение состояния -> событие -> broker -> consumer -> обработка
```


---

# Проектирование взаимодействия через брокер сообщений

## 1. Выбор брокера сообщений

Для проекта выбран:

```text
RabbitMQ
```

## 1.1 Почему RabbitMQ

RabbitMQ хорошо подходит для шестой лабораторной работы и hotel booking сервиса по следующим причинам:

```text
простая настройка через Docker
понятная модель exchange / queue / routing key
удобен для событий бизнес-домена
хорошо подходит для at-least-once доставки
легко показать producer и consumer в учебном проекте
есть management UI для проверки очередей и сообщений
```

Kafka больше подходит для больших потоков событий, event streaming и долгого хранения event log. В текущем проекте основной сценарий проще:

```text
API публикует событие о бронировании
consumer получает событие и обрабатывает уведомление
```

Поэтому RabbitMQ является более практичным выбором.

## 2. Общая схема RabbitMQ

Для событий проекта используется один topic exchange:

```text
hotel.events
```

Тип exchange:

```text
topic
```

Почему topic exchange:

```text
можно маршрутизировать события по предметной области
можно подписывать разные очереди на разные группы событий
можно удобно расширять систему новыми consumer'ами
```

Примеры routing keys:

```text
booking.created
booking.cancelled
hotel.created
user.registered
review.created
security.rate_limit_exceeded
```

## 3. Очереди

Для текущей реализации достаточно одной основной очереди:

```text
hotel.notifications
```

Она получает события:

```text
booking.created
booking.cancelled
user.registered
review.created
```

Для проектной архитектуры также описываются дополнительные очереди:

```text
hotel.analytics
hotel.read_model
hotel.security
```

## 3.1 hotel.notifications

Назначение:

```text
уведомления пользователей
```

Binding keys:

```text
booking.created
booking.cancelled
user.registered
review.created
```

Consumer:

```text
Notification Consumer
```

## 3.2 hotel.analytics

Назначение:

```text
аналитика событий системы
```

Binding keys:

```text
booking.*
hotel.*
user.*
review.*
security.*
```

Consumer:

```text
Analytics Consumer
```

## 3.3 hotel.read_model

Назначение:

```text
обновление read model и кешей
```

Binding keys:

```text
booking.created
booking.cancelled
hotel.created
review.created
```

Consumer:

```text
Read Model Consumer
```

## 3.4 hotel.security

Назначение:

```text
мониторинг подозрительной активности
```

Binding keys:

```text
security.rate_limit_exceeded
```

Consumer:

```text
Security / Monitoring Consumer
```

## 4. Формат сообщений

Все события публикуются в JSON-формате.

Content type:

```text
application/json
```

Базовый envelope:

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

## 4.1 Поля envelope

| Поле | Тип | Обязательное | Описание |
|---|---|---:|---|
| `eventId` | string | да | уникальный идентификатор события |
| `eventType` | string | да | тип события |
| `eventVersion` | number | да | версия схемы события |
| `occurredAt` | string/date-time | да | время возникновения события |
| `producer` | string | да | сервис-производитель |
| `correlationId` | string | нет | идентификатор запроса для трассировки |
| `payload` | object | да | данные события |

## 4.2 BookingCreated message

Routing key:

```text
booking.created
```

Message:

```json
{
  "eventId": "evt-booking-501-created",
  "eventType": "BookingCreated",
  "eventVersion": 1,
  "occurredAt": "2026-05-24T12:00:00Z",
  "producer": "hotel-booking-api",
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

## 4.3 BookingCancelled message

Routing key:

```text
booking.cancelled
```

Message:

```json
{
  "eventId": "evt-booking-501-cancelled",
  "eventType": "BookingCancelled",
  "eventVersion": 1,
  "occurredAt": "2026-05-24T13:00:00Z",
  "producer": "hotel-booking-api",
  "correlationId": "req-002",
  "payload": {
    "bookingId": 501,
    "userId": 101,
    "status": "cancelled"
  }
}
```

## 4.4 RateLimitExceeded message

Routing key:

```text
security.rate_limit_exceeded
```

Message:

```json
{
  "eventId": "evt-rate-limit-001",
  "eventType": "RateLimitExceeded",
  "eventVersion": 1,
  "occurredAt": "2026-05-24T13:10:00Z",
  "producer": "hotel-booking-api",
  "correlationId": "req-003",
  "payload": {
    "endpoint": "POST /bookings",
    "clientKey": "bookings:user:101",
    "limit": 10,
    "remaining": 0,
    "resetAfterSeconds": 12
  }
}
```

## 5. Гарантии доставки

Для текущего проекта выбрана гарантия:

```text
at-least-once
```

## 5.1 Почему at-least-once

At-least-once означает:

```text
сообщение будет доставлено потребителю один или несколько раз
```

Это практичный вариант для RabbitMQ и текущего проекта.

Плюсы:

```text
низкий риск потери события
простая реализация
подходит для уведомлений и аналитики
```

Минусы:

```text
consumer должен быть идемпотентным
одно событие может быть обработано повторно
```

## 5.2 Почему не exactly-once

Exactly-once сложнее обеспечить на практике, потому что нужно согласовать:

```text
запись в БД
публикацию события
обработку consumer'ом
подтверждение сообщения
```

Для booking-сервиса exactly-once избыточен. Вместо этого используется:

```text
at-least-once + idempotency по eventId
```

## 5.3 Идемпотентность consumer'ов

Consumer должен безопасно обрабатывать повторные сообщения.

Для этого у каждого сообщения есть:

```text
eventId
```

Consumer может хранить обработанные `eventId` и пропускать дубликаты.

Пример:

```text
если eventId уже обработан -> ack без повторного действия
если eventId новый -> обработать и сохранить eventId
```

## 6. RabbitMQ durability-настройки

Для production-подхода используются следующие настройки:

```text
durable exchange
durable queue
persistent messages
manual ack
prefetch limit
dead-letter queue
```

## 6.1 Durable exchange

Exchange должен переживать перезапуск RabbitMQ:

```text
hotel.events durable=true
```

## 6.2 Durable queues

Очереди должны переживать перезапуск RabbitMQ:

```text
hotel.notifications durable=true
hotel.analytics durable=true
hotel.read_model durable=true
hotel.security durable=true
```

## 6.3 Persistent messages

Сообщения должны публиковаться как persistent:

```text
delivery_mode = 2
```

Это снижает риск потери сообщений при перезапуске брокера.

## 6.4 Manual ack

Consumer подтверждает сообщение только после успешной обработки:

```text
basic_ack после обработки
basic_nack при ошибке
```

Если consumer упал до ack, RabbitMQ сможет доставить сообщение повторно.

## 6.5 Dead-letter queue

Для ошибок обработки можно добавить DLQ:

```text
hotel.notifications.dlq
```

Если сообщение несколько раз не удалось обработать, оно отправляется в DLQ для ручного анализа.

## 7. Routing table

| Event | Routing key | Exchange | Queues |
|---|---|---|---|
| `UserRegistered` | `user.registered` | `hotel.events` | `hotel.notifications`, `hotel.analytics` |
| `HotelCreated` | `hotel.created` | `hotel.events` | `hotel.analytics`, `hotel.read_model` |
| `BookingCreated` | `booking.created` | `hotel.events` | `hotel.notifications`, `hotel.analytics`, `hotel.read_model` |
| `BookingCancelled` | `booking.cancelled` | `hotel.events` | `hotel.notifications`, `hotel.analytics`, `hotel.read_model` |
| `ReviewCreated` | `review.created` | `hotel.events` | `hotel.notifications`, `hotel.analytics`, `hotel.read_model` |
| `RateLimitExceeded` | `security.rate_limit_exceeded` | `hotel.events` | `hotel.security`, `hotel.analytics` |

## 8. Поток publish / consume

## 8.1 Publish

```text
1. API успешно выполняет бизнес-операцию.
2. EventPublisher формирует JSON envelope.
3. EventPublisher публикует сообщение в hotel.events.
4. RabbitMQ маршрутизирует сообщение по routing key.
5. Сообщение попадает в одну или несколько очередей.
```

## 8.2 Consume

```text
1. Consumer подписан на очередь.
2. RabbitMQ доставляет сообщение.
3. Consumer читает eventType и payload.
4. Consumer выполняет обработку.
5. Consumer отправляет ack.
6. При ошибке consumer отправляет nack или сообщение уходит в DLQ.
```

## 9. Вывод по третьему пункту

Для проекта выбран RabbitMQ с topic exchange:

```text
exchange: hotel.events
type: topic
```

Основной реализуемый поток:

```text
POST /bookings
  -> BookingCreated
  -> routing key booking.created
  -> queue hotel.notifications
  -> Notification Consumer
```

И поток отмены:

```text
DELETE /bookings
  -> BookingCancelled
  -> routing key booking.cancelled
  -> queue hotel.notifications
  -> Notification Consumer
```

Гарантия доставки:

```text
at-least-once
```

Повторная обработка контролируется через:

```text
eventId
идемпотентность consumer'а
```


---

# Применение паттерна CQRS

## 1. Можно ли применить CQRS в системе

CQRS расшифровывается как Command Query Responsibility Segregation. Идея паттерна — разделить операции записи и операции чтения.

Для системы бронирования отелей CQRS применим, потому что в проекте есть разные требования к write- и read-операциям:

```text
write-операции должны быть строгими, консистентными и безопасными
read-операции должны быть быстрыми, кешируемыми и удобными для интерфейса
```

В booking-системе количество read-запросов обычно намного больше количества write-запросов. Пользователи часто ищут отели, открывают карточки и смотрят номера, но бронирование создают значительно реже.

Поэтому CQRS подходит для следующих частей проекта:

```text
каталог отелей
доступность номеров
бронирования пользователя
рейтинги и отзывы
аналитика
```

## 2. Write model

Write model отвечает за команды, которые меняют состояние системы.

В текущем проекте write model хранится в PostgreSQL и частично в MongoDB.

Основные свойства write model:

```text
строгая проверка данных
транзакции
валидация бизнес-правил
защита от конфликтов бронирования
публикация событий после успешной записи
```

## 2.1 Commands

| Command | Endpoint | Write model | Событие после успеха |
|---|---|---|---|
| `CreateUserCommand` | `POST /auth/register` | `users` | `UserRegistered` |
| `CreateHotelCommand` | `POST /hotels` | `hotels` | `HotelCreated` |
| `CreateBookingCommand` | `POST /bookings` | `bookings` | `BookingCreated` |
| `CancelBookingCommand` | `DELETE /bookings` | `bookings` | `BookingCancelled` |
| `CreateReviewCommand` | `POST /reviews` / `POST /mongo/reviews` | `reviews` | `ReviewCreated` |

## 2.2 Пример write flow: CreateBookingCommand

```text
1. Client отправляет POST /bookings.
2. API проверяет авторизацию.
3. RateLimiter проверяет лимит.
4. Storage проверяет доступность номера.
5. Storage создаёт запись в bookings.
6. Cache инвалидируется для rooms.
7. EventPublisher публикует BookingCreated.
8. Client получает ответ 201 Created.
```

Важно:

```text
событие публикуется только после успешной записи в write model
если запись в БД не прошла, событие не публикуется
```

## 3. Read model

Read model отвечает за быстрые запросы чтения. Она может иметь структуру, отличную от write model.

В текущем проекте read model частично уже присутствует:

```text
GET /hotels использует кешированный список отелей
GET /rooms использует кешированный список номеров
MongoDB bookings/reviews хранит документные данные
```

В Event-Driven архитектуре read model можно развить дальше:

```text
hotel_search_view
room_availability_view
user_bookings_view
hotel_rating_view
analytics_view
```

Эти модели можно хранить в MongoDB, Redis или отдельной read-таблице PostgreSQL.

## 3.1 Queries

| Query | Endpoint | Read model | Особенности |
|---|---|---|---|
| `FindUserByLoginQuery` | `GET /users?login=...` | `users` | поиск пользователя |
| `SearchUsersQuery` | `GET /users?query=...` | `users` | поиск по маске |
| `ListHotelsQuery` | `GET /hotels` | hotel catalog cache/read model | кешируется |
| `SearchHotelsByCityQuery` | `GET /hotels?city=...` | hotel search read model | кешируется |
| `ListRoomsQuery` | `GET /rooms?hotelId=...` | room catalog cache/read model | кешируется |
| `FindAvailableRoomsQuery` | `GET /rooms?...&checkIn=...` | availability read model | чувствителен к актуальности |
| `ListUserBookingsQuery` | `GET /bookings` | user bookings read model | история пользователя |
| `ListHotelReviewsQuery` | `GET /mongo/reviews?hotelId=...` | reviews read model | MongoDB/document model |

## 4. Как события синхронизируют write и read модели

События являются мостом между write model и read model.

Общий принцип:

```text
Command изменяет write model
после успешной записи публикуется event
consumer получает event
consumer обновляет read model
query читает уже оптимизированную read model
```

## 4.1 HotelCreated -> hotel search read model

Событие:

```text
HotelCreated
```

Что обновляет:

```text
hotel_search_view
кеш GET /hotels
кеш GET /hotels?city=...
```

Поток:

```text
POST /hotels
  -> запись в hotels
  -> HotelCreated
  -> Read Model Consumer
  -> обновление hotel_search_view
  -> очистка hotels:* cache keys
```

## 4.2 BookingCreated -> availability read model

Событие:

```text
BookingCreated
```

Что обновляет:

```text
room_availability_view
user_bookings_view
analytics_view
rooms cache
```

Поток:

```text
POST /bookings
  -> запись в bookings
  -> BookingCreated
  -> Read Model Consumer
  -> номер помечается занятым на период
  -> user_bookings_view получает новую бронь
  -> analytics_view увеличивает количество броней
```

## 4.3 BookingCancelled -> availability read model

Событие:

```text
BookingCancelled
```

Что обновляет:

```text
room_availability_view
user_bookings_view
analytics_view
rooms cache
```

Поток:

```text
DELETE /bookings
  -> status = cancelled
  -> BookingCancelled
  -> Read Model Consumer
  -> номер снова считается доступным на период
  -> user_bookings_view обновляет статус брони
  -> analytics_view увеличивает счётчик отмен
```

## 4.4 ReviewCreated -> rating read model

Событие:

```text
ReviewCreated
```

Что обновляет:

```text
hotel_rating_view
hotel_reviews_view
analytics_view
```

Поток:

```text
POST /reviews
  -> запись отзыва
  -> ReviewCreated
  -> Rating Consumer
  -> пересчёт averageRating/reviewsCount
```

## 5. Eventual consistency

При CQRS через события read model обновляется не мгновенно, а асинхронно. Поэтому система становится eventually consistent.

Это означает:

```text
write model обновляется сразу
event публикуется после записи
read model обновляется consumer'ом чуть позже
```

Для hotel booking сервиса это приемлемо для:

```text
рейтингов
аналитики
уведомлений
поисковых витрин
кешей каталога
```

Но для создания бронирования строгая проверка доступности должна оставаться в write model:

```text
POST /bookings всегда проверяет конфликт в PostgreSQL
нельзя полагаться только на read model доступности
```

## 6. CQRS в текущей реализации

В рамках шестой лабораторной работы CQRS применяется проектно и частично практически:

```text
commands остаются в REST API
write model остаётся в PostgreSQL
read-heavy endpoint'ы используют кеш
события BookingCreated/BookingCancelled публикуются в RabbitMQ
consumer обрабатывает события асинхронно
```

Минимальная практическая реализация:

```text
POST /bookings
  -> write model
  -> BookingCreated
  -> RabbitMQ
  -> Notification Consumer

DELETE /bookings
  -> write model
  -> BookingCancelled
  -> RabbitMQ
  -> Notification Consumer
```

Расширенная production-реализация:

```text
BookingCreated
  -> Notification Consumer
  -> Analytics Consumer
  -> Read Model Consumer
  -> Availability Consumer
```

## 7. Разделение команд и запросов в проекте

## 7.1 Commands

```text
POST /auth/register
POST /hotels
POST /bookings
DELETE /bookings
POST /reviews
POST /mongo/bookings
DELETE /mongo/bookings
POST /mongo/reviews
```

Команды:

```text
меняют состояние
выполняют валидацию
могут публиковать события
не кешируются
могут быть ограничены rate limiting
```

## 7.2 Queries

```text
GET /users
GET /hotels
GET /rooms
GET /bookings
GET /mongo/bookings
GET /mongo/reviews
```

Запросы:

```text
не меняют состояние
могут читать из read model
могут кешироваться
не публикуют бизнес-события
```

## 8. Преимущества CQRS для проекта

CQRS даёт следующие преимущества:

```text
read-операции можно оптимизировать отдельно от write-операций
можно строить быстрые read-модели для поиска отелей
можно кешировать каталожные данные
можно обновлять аналитику асинхронно
можно разгрузить основной PostgreSQL
можно независимо развивать consumer'ы
```

## 9. Риски CQRS

Основные риски:

```text
усложнение архитектуры
eventual consistency
необходимость идемпотентности consumer'ов
нужно следить за доставкой событий
нужно мониторить lag между write и read model
```

Для текущего проекта эти риски контролируются тем, что:

```text
основная бизнес-логика остаётся в одном API
RabbitMQ используется для ограниченного набора событий
строгая проверка бронирования остаётся в PostgreSQL
```

## 10. Итог по четвёртому пункту

CQRS применим в системе бронирования отелей.

Разделение:

```text
Commands:
- создание пользователя
- создание отеля
- создание бронирования
- отмена бронирования
- создание отзыва

Queries:
- поиск пользователя
- список отелей
- поиск отелей по городу
- список номеров
- бронирования пользователя
- отзывы отеля
```

События синхронизируют модели так:

```text
write model обновляется командой
после успешной записи публикуется event
consumer получает event
read model/cache/analytics обновляются асинхронно
```

В рамках реализации фокус остаётся на:

```text
BookingCreated
BookingCancelled
```

Эти события достаточно хорошо показывают CQRS и Event-Driven подход для booking-сервиса.


---

# Реализация простого Event-Driven сервиса

## 1. Что реализовано

В рамках пятого пункта шестой лабораторной добавлен простой Event-Driven сервис на RabbitMQ.

Добавленные файлы:

```text
event-service/producer.py
event-service/consumer.py
event-service/requirements.txt
```

Обновлены:

```text
docker-compose.yaml
docker-compose.yml
README.md
```

## 2. RabbitMQ в Docker

В `docker-compose.yaml` добавлен сервис:

```text
rabbitmq
```

Используется образ:

```text
rabbitmq:3.13-management
```

Порты:

```text
5672   AMQP protocol
15672  RabbitMQ Management UI
```

Доступ к management UI:

```text
http://localhost:15672
login: guest
password: guest
```

## 3. Producer

Файл:

```text
event-service/producer.py
```

Producer:

```text
подключается к RabbitMQ
создаёт topic exchange hotel.events
создаёт очередь hotel.notifications
публикует BookingCreated или BookingCancelled
использует persistent messages
```

По умолчанию producer публикует событие:

```text
BookingCreated
```

Routing key:

```text
booking.created
```

## 4. Consumer

Файл:

```text
event-service/consumer.py
```

Consumer:

```text
подключается к RabbitMQ
подписывается на hotel.notifications
обрабатывает booking.created и booking.cancelled
использует manual ack
хранит обработанные eventId в памяти для идемпотентности
```

В текущей реализации consumer имитирует Notification Service:

```text
BookingCreated   -> логирует подготовку уведомления о создании бронирования
BookingCancelled -> логирует подготовку уведомления об отмене бронирования
```

## 5. Как протестировать взаимодействие

Запуск окружения:

```bash
docker compose up --build
```

В отдельном терминале отправить тестовое событие:

```bash
docker compose run --rm event-producer
```

Опубликовать событие отмены:

```bash
docker compose run --rm \
  -e EVENT_TYPE=BookingCancelled \
  -e BOOKING_ID=501 \
  -e USER_ID=101 \
  event-producer
```

Посмотреть логи consumer:

```bash
docker compose logs -f event-consumer
```

Результат:

```text
Received BookingCreated event_id=...
Notification prepared: booking 501 created for user 101
```

или:

```text
Received BookingCancelled event_id=...
Notification prepared: booking 501 cancelled for user 101
```

## 6. Почему producer/consumer вынесены отдельно

Текущий C++/userver API уже содержит PostgreSQL, MongoDB, кеширование и rate limiting. Чтобы не сломать рабочий API и не добавлять нестабильные зависимости в сборку C++, Event-Driven часть реализована отдельным lightweight service на Python.

Такой подход сохраняет основной проект рабочим и при этом демонстрирует полный Event-Driven цикл:

```text
producer -> RabbitMQ -> queue -> consumer -> обработка события
```

В production-версии `EventPublisher` можно встроить прямо в `BookingsHandler`, чтобы `POST /bookings` публиковал `BookingCreated` автоматически после успешной записи в БД.
