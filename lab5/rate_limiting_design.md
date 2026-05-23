# Rate limiting design Итог

Основной endpoint для реализации rate limiting:

```text
POST /bookings
```

Алгоритм:

```text
Token Bucket
```

Лимит для реализации:

```text
10 запросов в минуту на пользователя
```

Причина:

```text
POST /bookings — дорогая write-операция: проверяет доступность номера и пишет бронирование в PostgreSQL.
```

При превышении лимита API должен вернуть:

```text
429 Too Many Requests
```

И заголовки:

```text
X-RateLimit-Limit
X-RateLimit-Remaining
X-RateLimit-Reset
```

Дополнительно в проектировании описаны лимиты для:

```text
POST /auth/login
POST /auth/register
GET /hotels
GET /rooms
```

Полное описание находится в `performance_design.md`.
