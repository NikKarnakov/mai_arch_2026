# Лабораторная работа №3

## Результат

Итоговые файлы:

```text
schema.sql          # создание схемы БД: таблицы, ограничения, индексы
data.sql            # тестовые данные
queries.sql         # SQL-запросы для операций сервиса
optimization.md     # анализ и оптимизация запросов через EXPLAIN
README.md           # описание схемы и инструкция запуска
Dockerfile          # сборка API
docker-compose.yaml # запуск API и PostgreSQL
```

Также в проекте есть дополнительные директории:

```text
db/                 # SQL-файлы для автоматической инициализации PostgreSQL в Docker
src/                # C++/userver API
configs/            # конфиги userver
indexes.md          # подробное описание индексов
partitioning.md     # стратегия партиционирования bookings
README_API.md       # примеры HTTP-запросов к API
```

## Схема базы данных

В базе данных используются 5 основных таблиц:

```text
users
hotels
rooms
bookings
reviews
```

### users

Хранит пользователей сервиса.

Основные поля:

- `id` — первичный ключ;
- `first_name`, `last_name` — имя и фамилия;
- `login` — уникальный логин;
- `email` — уникальная почта;
- `password_hash` — хеш пароля;
- `created_at` — дата создания.

Ограничения:

- `login` уникален;
- `email` уникален;
- `login` не короче 3 символов;
- `email` должен содержать `@`.

### hotels

Хранит отели.

Основные поля:

- `id` — первичный ключ;
- `name` — название отеля;
- `city` — город;
- `address` — адрес;
- `stars` — количество звёзд;
- `description` — описание.

Ограничения:

- `stars` от 1 до 5;
- пара `name + address` уникальна.

### rooms

Хранит номера отелей.

Основные поля:

- `id` — первичный ключ;
- `hotel_id` — внешний ключ на `hotels`;
- `room_number` — номер комнаты;
- `room_type` — тип номера;
- `capacity` — вместимость;
- `price_per_night` — цена за ночь;
- `is_available` — технический флаг доступности.

Ограничения:

- `hotel_id` ссылается на `hotels(id)`;
- вместимость больше 0;
- цена больше 0;
- тип номера ограничен списком: `standard`, `comfort`, `family`, `luxury`;
- номер комнаты уникален внутри одного отеля.

### bookings

Хранит бронирования.

Основные поля:

- `id` — первичный ключ;
- `user_id` — внешний ключ на `users`;
- `room_id` — внешний ключ на `rooms`;
- `check_in` — дата заезда;
- `check_out` — дата выезда;
- `guests_count` — количество гостей;
- `status` — статус бронирования;
- `total_price` — итоговая цена;
- `created_at` — дата создания.

Ограничения:

- `check_out > check_in`;
- `guests_count > 0`;
- `total_price >= 0`;
- статус ограничен списком: `created`, `confirmed`, `cancelled`, `completed`.

### reviews

Хранит отзывы.

Основные поля:

- `id` — первичный ключ;
- `user_id` — внешний ключ на `users`;
- `hotel_id` — внешний ключ на `hotels`;
- `booking_id` — необязательная ссылка на бронирование;
- `rating` — оценка;
- `comment` — текст отзыва.

Ограничения:

- `rating` от 1 до 5;
- пользователь не может несколько раз оставить отзыв на одну и ту же связку `user_id + hotel_id + booking_id`.

## Связи между таблицами

```text
users  1 --- N bookings
hotels 1 --- N rooms
rooms  1 --- N bookings
users  1 --- N reviews
hotels 1 --- N reviews
bookings 1 --- 0..1 reviews
```

## Индексы

Первичные ключи и уникальные ограничения создают индексы автоматически.

Дополнительно созданы индексы:

```sql
CREATE INDEX idx_rooms_hotel_id
    ON rooms (hotel_id);

CREATE INDEX idx_rooms_hotel_type_available
    ON rooms (hotel_id, room_type, is_available);

CREATE INDEX idx_hotels_city
    ON hotels (city);

CREATE INDEX idx_hotels_city_stars
    ON hotels (city, stars);

CREATE INDEX idx_bookings_user_id
    ON bookings (user_id);

CREATE INDEX idx_bookings_room_id
    ON bookings (room_id);

CREATE INDEX idx_bookings_room_dates_active
    ON bookings (room_id, check_in, check_out)
    WHERE status IN ('created', 'confirmed');

CREATE INDEX idx_bookings_status
    ON bookings (status);

CREATE INDEX idx_reviews_hotel_id
    ON reviews (hotel_id);

CREATE INDEX idx_reviews_user_id
    ON reviews (user_id);

CREATE INDEX idx_reviews_booking_id
    ON reviews (booking_id);
```

Самый важный индекс — `idx_bookings_room_dates_active`. Он используется при проверке, свободен ли номер на выбранный диапазон дат.

## Запуск проекта

### 1. Запуск PostgreSQL и API

```bash
docker compose up --build
```

PostgreSQL будет доступен на порту `5432`, API — на порту `8080`.

### 2. Проверка API

```bash
curl http://localhost:8080/ping
```

### 3. Подключение к базе данных

```bash
psql postgresql://hotel_user:hotel_password@localhost:5432/hotel_booking
```

### 4. Проверка количества данных

```sql
SELECT COUNT(*) FROM users;
SELECT COUNT(*) FROM hotels;
SELECT COUNT(*) FROM rooms;
SELECT COUNT(*) FROM bookings;
SELECT COUNT(*) FROM reviews;
```

## Примеры API-запросов

### Регистрация

```bash
curl -X POST http://localhost:8080/auth/register \
  -H "Content-Type: application/json" \
  -d '{"firstName":"Roman","lastName":"Ivanov","login":"romanx","email":"romanx@example.com","password":"12345"}'
```

### Авторизация

```bash
curl -X POST http://localhost:8080/auth/login \
  -H "Content-Type: application/json" \
  -d '{"login":"romanx","password":"12345"}'
```

### Поиск отелей

```bash
curl "http://localhost:8080/hotels?city=Moscow&minStars=4"
```

### Просмотр номеров

```bash
curl "http://localhost:8080/rooms?hotelId=1"
```

### Поиск свободных номеров

```bash
curl "http://localhost:8080/rooms?hotelId=1&roomType=standard&guests=2&checkIn=2026-06-10&checkOut=2026-06-12"
```

### Создание бронирования

```bash
curl -X POST http://localhost:8080/bookings \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer <TOKEN_FROM_LOGIN>" \
  -d '{"roomId":1,"checkIn":"2026-06-10","checkOut":"2026-06-12","guestsCount":2}'
```

### Просмотр своих бронирований

```bash
curl http://localhost:8080/bookings \
  -H "Authorization: Bearer <TOKEN_FROM_LOGIN>"
```

### Отмена бронирования

```bash
curl -X DELETE http://localhost:8080/bookings \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer <TOKEN_FROM_LOGIN>" \
  -d '{"bookingId":1}'
```

## Оптимизация запросов

Файл `optimization.md` содержит:

- SQL-запросы с `EXPLAIN ANALYZE`;
- сравнение поведения до и после индексов;
- объяснение, почему выбраны именно эти индексы;
- описание оптимизации поиска свободных номеров через `NOT EXISTS`;
- описание частичного индекса для активных бронирований.

Для сравнения планов до индексов добавлен файл:

```text
db/schema_before_indexes.sql
```

## Партиционирование

Партиционирование вынесено в отдельный файл:

```text
partitioning.md
db/partitioning.sql
```

Для production-сценария предложено партиционировать таблицу `bookings` по `check_in` поквартально. В основной `schema.sql` партиционирование не включено, чтобы проект оставался простым и запускался без дополнительных действий.

## Подключение API к PostgreSQL

Старые in-memory хранилища заменены на слой `Storage`, который работает с PostgreSQL.

Основной поток запроса:

```text
HTTP handler -> Storage -> PostgreSQL
```

Файлы реализации:

```text
src/main.cpp
src/handlers.hpp
src/handlers.cpp
src/storage.hpp
src/storage.cpp
src/auth.hpp
src/auth.cpp
```

Конфигурация userver находится в:

```text
configs/static_config.yaml
configs/config_vars.docker.yaml
```

## Примечание 

В лабораторной работе используется простой формат токена и демонстрационное хеширование пароля. Для production-версии это нужно заменить на JWT/opaque sessions и bcrypt/Argon2.
