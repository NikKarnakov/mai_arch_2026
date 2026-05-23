# Cache design summИтогary

Основная стратегия кеширования для проекта — Cache-Aside.

Кешируемые endpoint'ы:

```text
GET /hotels
GET /rooms?hotelId=...
```

TTL:

```text
60 секунд
```

Инвалидация:

```text
POST /hotels     -> удалить hotels:*
POST /bookings   -> удалить rooms:*
DELETE /bookings -> удалить rooms:*
```

Полное описание находится в `performance_design.md`.
