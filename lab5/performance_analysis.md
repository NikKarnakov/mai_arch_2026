# Performance analysis Итог

## Что улучшает кеширование

Кеширование снижает количество повторных обращений к PostgreSQL для endpoint'ов:

```text
GET /hotels
GET /rooms?hotelId=...
```

Повторные запросы обслуживаются из in-memory cache, поэтому уменьшается время ответа и нагрузка на базу данных.

## Что улучшает rate limiting

Rate limiting защищает endpoint:

```text
POST /bookings
```

Он ограничивает частоту дорогих write-операций и предотвращает перегрузку PostgreSQL при большом количестве повторных запросов.

## Основные метрики

```text
cache_hits_total
cache_misses_total
cache_hit_rate
http_request_duration_ms
db_queries_total
http_429_total
rate_limit_allowed_total
rate_limit_rejected_total
```

## Формула hit rate

```text
cache_hit_rate = cache_hits / (cache_hits + cache_misses)
```

## Проверка кеша

```bash
curl -i "http://localhost:8080/hotels?city=Moscow&minStars=4"
curl -i "http://localhost:8080/hotels?city=Moscow&minStars=4"
```

Ожидаемо:

```text
первый запрос -> X-Cache: MISS
второй запрос -> X-Cache: HIT
```

## Проверка rate limiting

```bash
TOKEN="<TOKEN_FROM_LOGIN>"

for i in {1..12}; do
  curl -i -X POST http://localhost:8080/bookings \
    -H "Content-Type: application/json" \
    -H "Authorization: Bearer $TOKEN" \
    -d '{"roomId":1,"checkIn":"2026-12-10","checkOut":"2026-12-11","guestsCount":1}'
done
```

После превышения лимита API должен вернуть:

```text
429 Too Many Requests
```
