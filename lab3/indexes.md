# Индексы базы данных

Третий пункт лабораторной работы посвящён индексам. Ниже описаны частые запросы сервиса бронирования отелей и индексы, которые помогают PostgreSQL выполнять эти запросы быстрее.

## Частые операции API

В сервисе чаще всего выполняются следующие операции:

1. Авторизация пользователя по `login`.
2. Поиск отелей по городу.
3. Фильтрация отелей по городу и категории.
4. Получение списка номеров конкретного отеля.
5. Фильтрация доступных номеров по отелю и типу номера.
6. Создание бронирования с проверкой занятости номера на даты.
7. Просмотр бронирований конкретного пользователя.
8. Отмена бронирования.
9. Получение отзывов по отелю.
10. Получение отзывов пользователя.

## Автоматические индексы

PostgreSQL автоматически создаёт индексы для `PRIMARY KEY` и `UNIQUE` ограничений.

В схеме лабораторной работы автоматически индексируются:

- `users.id`
- `users.login`
- `users.email`
- `hotels.id`
- `hotels(name, address)`
- `rooms.id`
- `rooms(hotel_id, room_number)`
- `bookings.id`
- `reviews.id`
- `reviews(user_id, hotel_id, booking_id)`

Поэтому отдельные индексы на эти поля вручную не добавляются.

## Явносозданные индексы

### `idx_rooms_hotel_id`

```sql
CREATE INDEX idx_rooms_hotel_id
    ON rooms (hotel_id);
```

Нужен для связи `hotels -> rooms` и для запроса списка номеров выбранного отеля.

Типичный запрос:

```sql
SELECT *
FROM rooms
WHERE hotel_id = $1;
```

---

### `idx_rooms_hotel_type_available`

```sql
CREATE INDEX idx_rooms_hotel_type_available
    ON rooms (hotel_id, room_type, is_available);
```

Нужен для фильтрации номеров внутри конкретного отеля по типу и признаку доступности.

Типичный запрос:

```sql
SELECT *
FROM rooms
WHERE hotel_id = $1
  AND room_type = $2
  AND is_available = TRUE;
```

---

### `idx_hotels_city`

```sql
CREATE INDEX idx_hotels_city
    ON hotels (city);
```

Нужен для поиска отелей по городу.

Типичный запрос:

```sql
SELECT *
FROM hotels
WHERE city = $1;
```

---

### `idx_hotels_city_stars`

```sql
CREATE INDEX idx_hotels_city_stars
    ON hotels (city, stars);
```

Нужен для фильтрации отелей по городу и количеству звёзд. Составной индекс выбран потому что пользователь обычно сначала выбирает город, а затем уточняет категорию отеля.

Типичный запрос:

```sql
SELECT *
FROM hotels
WHERE city = $1
  AND stars >= $2;
```

---

### `idx_bookings_user_id`

```sql
CREATE INDEX idx_bookings_user_id
    ON bookings (user_id);
```

Нужен для быстрого получения всех бронирований пользователя.

Типичный запрос:

```sql
SELECT *
FROM bookings
WHERE user_id = $1
ORDER BY created_at DESC;
```

---

### `idx_bookings_room_id`

```sql
CREATE INDEX idx_bookings_room_id
    ON bookings (room_id);
```

Нужен для связи `rooms -> bookings` и анализа бронирований по конкретному номеру.

Типичный запрос:

```sql
SELECT *
FROM bookings
WHERE room_id = $1;
```

---

### `idx_bookings_room_dates_active`

```sql
CREATE INDEX idx_bookings_room_dates_active
    ON bookings (room_id, check_in, check_out)
    WHERE status IN ('created', 'confirmed');
```

Это самый важный индекс для бронирования. Он помогает проверять, свободен ли номер на выбранный диапазон дат. Индекс сделан частичным потому что отменённые и завершённые бронирования не должны блокировать новые брони.

Типичный запрос:

```sql
SELECT 1
FROM bookings
WHERE room_id = $1
  AND status IN ('created', 'confirmed')
  AND check_in < $3
  AND check_out > $2
LIMIT 1;
```

---

### `idx_bookings_status`

```sql
CREATE INDEX idx_bookings_status
    ON bookings (status);
```

Нужен для административных и сервисных выборок по статусу бронирования.

Типичный запрос:

```sql
SELECT *
FROM bookings
WHERE status = 'confirmed';
```

---

### `idx_reviews_hotel_id`

```sql
CREATE INDEX idx_reviews_hotel_id
    ON reviews (hotel_id);
```

Нужен для страницы отеля, где отображаются отзывы.

Типичный запрос:

```sql
SELECT *
FROM reviews
WHERE hotel_id = $1
ORDER BY created_at DESC;
```

---

### `idx_reviews_user_id`

```sql
CREATE INDEX idx_reviews_user_id
    ON reviews (user_id);
```

Нужен для получения отзывов конкретного пользователя.

Типичный запрос:

```sql
SELECT *
FROM reviews
WHERE user_id = $1;
```

---

### `idx_reviews_booking_id`

```sql
CREATE INDEX idx_reviews_booking_id
    ON reviews (booking_id);
```

Нужен для связи отзыва с конкретным бронированием.

Типичный запрос:

```sql
SELECT *
FROM reviews
WHERE booking_id = $1;
```

## Ответ на вопрос - почнму не создаются лишние индексы

Индексы ускоряют чтение, но замедляют вставку и обновление данных. Поэтому в существуюищую схему не добавлены индексы на каждую колонку подряд.

Например, отдельный индекс на `rooms.price_per_night` пока не нужен, потому что в текущем API нет основной операции поиска только по цене. Если позже сделать фильтр `price_from / price_to`, можно добавить составной индекс:

```sql
CREATE INDEX idx_rooms_hotel_price
    ON rooms (hotel_id, price_per_night);
```

Также пока не создаётся индекс на `created_at`, потому что он полезен только при больших объёмах данных и частой сортировке/архивной аналитике.
