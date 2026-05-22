# Оптимизация запросов

В этом файле описаны SQL-запросы для основных операций сервиса бронирования отелей, анализ через `EXPLAIN` и сравнение поведения запросов до и после добавления индексов.

PostgreSQL строит план выполнения запроса автоматически. Команда `EXPLAIN` показывает, как именно планировщик собирается выполнять запрос, а `EXPLAIN ANALYZE` дополнительно запускает запрос и показывает фактическое время выполнения.

## Как запустить анализ

### Вариант 1. Схема без пользовательских индексов

Для сравнения можно временно поднять базу на схеме без явных индексов:

```bash
psql postgresql://hotel_user:hotel_password@localhost:5432/hotel_booking \
  -f db/schema_before_indexes.sql

psql postgresql://hotel_user:hotel_password@localhost:5432/hotel_booking \
  -f db/data.sql
```

После этого выполнить `EXPLAIN ANALYZE` из разделов ниже.

### Вариант 2. Схема с индексами

Обычный вариант запуска задания лабораторных работ использует `db/schema.sql`. В нём уже есть индексы:

```bash
psql postgresql://hotel_user:hotel_password@localhost:5432/hotel_booking \
  -f db/schema.sql

psql postgresql://hotel_user:hotel_password@localhost:5432/hotel_booking \
  -f db/data.sql
```

Для более четкого сравнения планов стоит выполнить:

```sql
ANALYZE;
```

На маленьком наборе из 10 записей PostgreSQL иногда выбирает `Seq Scan`, даже если индекс существует. Это ок: для маленькой таблицы последовательное чтение может быть дешевле. На реальных данных, когда таблицы содержат тысячи и более строк, индексы начинают использоваться заметно чаще.

---

## 1. Авторизация пользователя

### Запрос

```sql
EXPLAIN ANALYZE
SELECT id, login, password_hash
FROM users
WHERE login = 'ivanp';
```

### До оптимизации

Для `users.login` отдельный индекс вручную не создавался, но поле имеет ограничение `UNIQUE`.

```sql
login VARCHAR(80) NOT NULL UNIQUE
```

PostgreSQL автоматически создаёт уникальный индекс для этого ограничения.

### После оптимизации

Дополнительная оптимизация не требуется. Поиск пользователя по логину уже работает через уникальный индекс.

Ожидаемый план на больших данных:

```text
Index Scan using users_login_key on users
  Index Cond: ((login)::text = 'ivanp'::text)
```

---

## 2. Поиск отелей по городу

### Запрос

```sql
EXPLAIN ANALYZE
SELECT id, name, city, address, stars, description
FROM hotels
WHERE city = 'Moscow';
```

### До оптимизации

Без индекса PostgreSQL должен просматривать таблицу `hotels` целиком:

```text
Seq Scan on hotels
  Filter: ((city)::text = 'Moscow'::text)
```

### Индекс

```sql
CREATE INDEX idx_hotels_city
    ON hotels (city);
```

### После оптимизации

На большом объёме данных планировщик может использовать индекс:

```text
Index Scan using idx_hotels_city on hotels
  Index Cond: ((city)::text = 'Moscow'::text)
```

### Обоснование

Поиск отелей по городу — одна из основных операций booking-сервиса. Пользователь почти всегда начинает поиск с выбора города.

---

## 3. Фильтрация отелей по городу и категории

### Запрос

```sql
EXPLAIN ANALYZE
SELECT id, name, city, address, stars, description
FROM hotels
WHERE city = 'Moscow'
  AND stars >= 4
ORDER BY stars DESC, name;
```

### До оптимизации

Без составного индекса PostgreSQL может читать все строки и затем фильтровать их:

```text
Seq Scan on hotels
  Filter: (((city)::text = 'Moscow'::text) AND (stars >= 4))
```

### Индекс

```sql
CREATE INDEX idx_hotels_city_stars
    ON hotels (city, stars);
```

### После оптимизации

```text
Index Scan using idx_hotels_city_stars on hotels
  Index Cond: (((city)::text = 'Moscow'::text) AND (stars >= 4))
```

### Обоснование

Составной индекс полезен, потому что запрос фильтрует сразу по двум полям. Первым стоит `city`, так как это базовое условие поиска.

---

## 4. Получение номеров отеля

### Запрос

```sql
EXPLAIN ANALYZE
SELECT id, hotel_id, room_number, room_type, capacity, price_per_night, is_available
FROM rooms
WHERE hotel_id = 1
ORDER BY room_number;
```

### До оптимизации

```text
Seq Scan on rooms
  Filter: (hotel_id = 1)
```

### Индекс

```sql
CREATE INDEX idx_rooms_hotel_id
    ON rooms (hotel_id);
```

### После оптимизации

```text
Index Scan using idx_rooms_hotel_id on rooms
  Index Cond: (hotel_id = 1)
```

### Обоснование

Связь `hotels -> rooms` используется постоянно: страница отеля должна быстро показывать список номеров.

---

## 5. Поиск доступных номеров

### Запрос

```sql
EXPLAIN ANALYZE
SELECT r.id,
       r.hotel_id,
       r.room_number,
       r.room_type,
       r.capacity,
       r.price_per_night
FROM rooms r
WHERE r.hotel_id = 1
  AND r.room_type = 'standard'
  AND r.capacity >= 2
  AND r.is_available = TRUE
  AND NOT EXISTS (
      SELECT 1
      FROM bookings b
      WHERE b.room_id = r.id
        AND b.status IN ('created', 'confirmed')
        AND b.check_in < DATE '2026-06-05'
        AND b.check_out > DATE '2026-06-01'
  )
ORDER BY r.price_per_night, r.room_number;
```

### До оптимизации

Без индексов PostgreSQL вынужден отдельно просматривать `rooms` и проверять бронирования через подзапрос:

```text
Seq Scan on rooms r
  Filter: ((hotel_id = 1) AND ...)
SubPlan
  -> Seq Scan on bookings b
```

### Индексы

```sql
CREATE INDEX idx_rooms_hotel_type_available
    ON rooms (hotel_id, room_type, is_available);

CREATE INDEX idx_bookings_room_dates_active
    ON bookings (room_id, check_in, check_out)
    WHERE status IN ('created', 'confirmed');
```

### После оптимизации

```text
Index Scan using idx_rooms_hotel_type_available on rooms r
  Index Cond: ((hotel_id = 1) AND ((room_type)::text = 'standard'::text) AND (is_available = true))

Index Only Scan or Index Scan using idx_bookings_room_dates_active on bookings b
  Index Cond: ((room_id = r.id) AND (check_in < '2026-06-05'::date) AND (check_out > '2026-06-01'::date))
```

### Обоснование

Это один из самых важных запросов системы. Он проверяет, можно ли показать номер пользователю как доступный. Частичный индекс по активным бронированиям уменьшает размер индекса и не учитывает `cancelled`/`completed` брони.

---

## 6. Проверка конфликта бронирования

### Запрос

```sql
EXPLAIN ANALYZE
SELECT EXISTS (
    SELECT 1
    FROM bookings
    WHERE room_id = 1
      AND status IN ('created', 'confirmed')
      AND check_in < DATE '2026-06-05'
      AND check_out > DATE '2026-06-01'
) AS has_conflict;
```

### До оптимизации

```text
Seq Scan on bookings
  Filter: ((room_id = 1) AND ...)
```

### Индекс

```sql
CREATE INDEX idx_bookings_room_dates_active
    ON bookings (room_id, check_in, check_out)
    WHERE status IN ('created', 'confirmed');
```

### После оптимизации

```text
Index Scan using idx_bookings_room_dates_active on bookings
  Index Cond: ((room_id = 1) AND (check_in < '2026-06-05'::date) AND (check_out > '2026-06-01'::date))
```

### Обоснование

Проверка пересечения дат выполняется перед созданием бронирования. Запрос переписан через `EXISTS`, потому что сервису нужен только факт наличия конфликта, а не все строки.

Условие пересечения интервалов:

```sql
check_in < requested_check_out
AND check_out > requested_check_in
```

Такой вариант короче и надёжнее, чем несколько отдельных условий для разных вариантов пересечения.

---

## 7. Просмотр бронирований пользователя

### Запрос

```sql
EXPLAIN ANALYZE
SELECT b.id,
       b.status,
       b.check_in,
       b.check_out,
       b.guests_count,
       b.total_price,
       b.created_at,
       h.name AS hotel_name,
       h.city AS hotel_city,
       r.room_number,
       r.room_type
FROM bookings b
JOIN rooms r ON r.id = b.room_id
JOIN hotels h ON h.id = r.hotel_id
WHERE b.user_id = 1
ORDER BY b.created_at DESC;
```

### До оптимизации

```text
Seq Scan on bookings b
  Filter: (user_id = 1)
Nested Loop
  -> Index Scan using rooms_pkey
  -> Index Scan using hotels_pkey
```

### Индекс

```sql
CREATE INDEX idx_bookings_user_id
    ON bookings (user_id);
```

### После оптимизации

```text
Index Scan using idx_bookings_user_id on bookings b
  Index Cond: (user_id = 1)
Nested Loop
  -> Index Scan using rooms_pkey on rooms r
  -> Index Scan using hotels_pkey on hotels h
```

### Обоснование

Личный кабинет пользователя часто показывает историю бронирований. Индекс по `bookings.user_id` ускоряет эту выборку.

---

## 8. Отмена бронирования

### Запрос

```sql
EXPLAIN ANALYZE
UPDATE bookings
SET status = 'cancelled'
WHERE id = 1
  AND user_id = 1
  AND status IN ('created', 'confirmed')
RETURNING id, status;
```

### До и после оптимизации

Основной поиск идёт по `bookings.id`, а `id` является первичным ключом. PostgreSQL автоматически создаёт индекс для `PRIMARY KEY`.

Ожидаемый план:

```text
Index Scan using bookings_pkey on bookings
  Index Cond: (id = 1)
  Filter: ((user_id = 1) AND ((status)::text = ANY (...)))
```

### Обоснование

Дополнительный индекс не нужен: отмена брони происходит по уникальному идентификатору. Проверка `user_id` нужна для безопасности, чтобы пользователь не отменил чужое бронирование.

---

## 9. Отзывы по отелю

### Запрос

```sql
EXPLAIN ANALYZE
SELECT r.id,
       r.rating,
       r.comment,
       r.created_at,
       u.first_name,
       u.last_name
FROM reviews r
JOIN users u ON u.id = r.user_id
WHERE r.hotel_id = 1
ORDER BY r.created_at DESC;
```

### До оптимизации

```text
Seq Scan on reviews r
  Filter: (hotel_id = 1)
```

### Индекс

```sql
CREATE INDEX idx_reviews_hotel_id
    ON reviews (hotel_id);
```

### После оптимизации

```text
Index Scan using idx_reviews_hotel_id on reviews r
  Index Cond: (hotel_id = 1)
```

### Обоснование

Отзывы отображаются на странице отеля, поэтому выборка по `hotel_id` будет частой.

---

## 10. Средний рейтинг отеля

### Запрос

```sql
EXPLAIN ANALYZE
SELECT h.id,
       h.name,
       COUNT(r.id) AS reviews_count,
       ROUND(AVG(r.rating)::numeric, 2) AS average_rating
FROM hotels h
LEFT JOIN reviews r ON r.hotel_id = h.id
WHERE h.id = 1
GROUP BY h.id, h.name;
```

### До оптимизации

```text
Index Scan using hotels_pkey on hotels h
Seq Scan on reviews r
  Filter: (hotel_id = 1)
```

### Индекс

```sql
CREATE INDEX idx_reviews_hotel_id
    ON reviews (hotel_id);
```

### После оптимизации

```text
Index Scan using hotels_pkey on hotels h
Index Scan using idx_reviews_hotel_id on reviews r
  Index Cond: (hotel_id = 1)
```

### Обоснование

Средний рейтинг может отображаться в карточке отеля. Индекс по `reviews.hotel_id` ускоряет агрегацию отзывов конкретного отеля.

---

## Итог сравнения

| Операция | До оптимизации | После оптимизации |
|---|---|---|
| Поиск пользователя по логину | Автоматический UNIQUE индекс | Дополнительный индекс не нужен |
| Поиск отелей по городу | `Seq Scan` | `Index Scan` по `idx_hotels_city` |
| Фильтр отелей по городу и звёздам | `Seq Scan` | `Index Scan` по `idx_hotels_city_stars` |
| Список номеров отеля | `Seq Scan` | `Index Scan` по `idx_rooms_hotel_id` |
| Поиск доступных номеров | `Seq Scan` + подзапрос по bookings | Индексы `idx_rooms_hotel_type_available` и `idx_bookings_room_dates_active` |
| Проверка конфликта брони | `Seq Scan` по bookings | Частичный индекс `idx_bookings_room_dates_active` |
| Брони пользователя | `Seq Scan` по bookings | `Index Scan` по `idx_bookings_user_id` |
| Отмена брони | `Index Scan` по PK | Дополнительный индекс не нужен |
| Отзывы отеля | `Seq Scan` | `Index Scan` по `idx_reviews_hotel_id` |
| Средний рейтинг отеля | `Seq Scan` по reviews | `Index Scan` по `idx_reviews_hotel_id` |

## Вывод

Основная оптимизация была сделана не за счёт добавления индексов на все поля подряд, а за счёт индексов под реальные операции API. Самый важный запрос — поиск свободных номеров — оптимизирован двумя индексами: составным индексом по номерам и частичным индексом по активным бронированиям.

Такая схема хорошо подходит для подключения к userver API: запросы из `queries.sql` можно перенести в repository-слой сервиса и выполнять через PostgreSQL-компонент.
