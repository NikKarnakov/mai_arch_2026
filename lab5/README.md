# Лабораторная работа №5

В пятой лабораторной работе реализованы оптимизации производительности: кеширование read-heavy endpoint'ов и rate limiting для дорогой write-операции создания бронирования.

## Результат

Файлы лабораторной работы:

```text
performance_design.md       # стратегия кеширования, rate limiting и анализ производительности
README.md                   # описание проекта и реализованных оптимизаций
Dockerfile                  # сборка API
docker-compose.yaml         # запуск API, PostgreSQL и MongoDB
```

Код реализации кеширования и rate limiting:

```text
src/response_cache.hpp
src/response_cache.cpp
src/rate_limiter.hpp
src/rate_limiter.cpp
```

Изменённые файлы основного API:

```text
src/main.cpp
src/handlers.hpp
src/handlers.cpp
configs/static_config.yaml
CMakeLists.txt
```

Дополнительные документы:

```text
cache_design.md
rate_limiting_design.md
performance_analysis.md
```

## Архитектура после 5-й лабораторной

Упрощённый поток запроса:

```text
HTTP request
    -> userver handler
        -> ResponseCache для GET /hotels и GET /rooms
        -> RateLimiter для POST /bookings
        -> Storage
            -> PostgreSQL / MongoDB
```

Компоненты:

```text
Storage          # PostgreSQL-операции
MongoStorage     # MongoDB-операции
ResponseCache    # in-memory cache
RateLimiter      # Token Bucket rate limiter
```

## Пункт 1. Анализ производительности

В рамках анализа были выделены hot paths — операции, которые вызываются чаще всего:

```text
GET /hotels
GET /hotels?city=...
GET /rooms?hotelId=...
POST /auth/login
POST /bookings
```

Потенциально медленные операции:

```text
поиск свободных номеров по датам
создание бронирования
получение большой истории бронирований пользователя
агрегации по отзывам и выручке
```

Целевые ориентиры по времени ответа:

```text
GET /hotels без кеша       до 150 мс
GET /hotels с кешем        до 30 мс
GET /rooms без кеша        до 120 мс
GET /rooms с кешем         до 30 мс
POST /bookings             до 300 мс
```

Подробное описание находится в:

```text
performance_design.md
```

## Пункт 2. Проектирование стратегии кеширования

Для кеширования выбрана стратегия:

```text
Cache-Aside / Lazy Loading
```

Почему именно она:

```text
простая реализация
подходит для read-heavy endpoint'ов
не ломает текущую архитектуру
при cache miss данные читаются из PostgreSQL
при недоступности кеша сервис может продолжить работать через БД
```

Кешируемые данные:

```text
список отелей
результаты поиска отелей по городу
список номеров конкретного отеля
```

TTL:

```text
60 секунд
```

Endpoint'ы:

```text
GET /hotels
GET /hotels?city=...
GET /rooms?hotelId=...
```

Не кешируются:

```text
POST /auth/login
POST /bookings
DELETE /bookings
GET /rooms?hotelId=...&checkIn=...&checkOut=...
```

Причина: эти операции либо связаны с безопасностью, либо меняют состояние системы, либо зависят от актуальной доступности номеров.

## Пункт 3. Реализация кеширования

Добавлен компонент:

```text
src/response_cache.hpp
src/response_cache.cpp
```

`ResponseCache` хранит готовые JSON-ответы в памяти процесса API.

Основные методы:

```text
Get(key)
Put(key, value, ttl)
InvalidatePrefix(prefix)
Clear()
```

Кеширование применено минимум для двух endpoint'ов:

```text
GET /hotels
GET /rooms?hotelId=...
```

Ключи кеша:

```text
hotels:city:{city}:min_stars:{minStars}
rooms:hotel:{hotelId}
```

Инвалидация кеша:

```text
POST /hotels     -> удаляет hotels:*
POST /bookings   -> удаляет rooms:*
DELETE /bookings -> удаляет rooms:*
```

Для проверки добавлены диагностические заголовки:

```text
X-Cache: MISS
X-Cache: HIT
X-Cache-Invalidated: hotels
X-Cache-Invalidated: rooms
```

Пример проверки:

```bash
curl -i "http://localhost:8080/hotels?city=Moscow&minStars=4"
curl -i "http://localhost:8080/hotels?city=Moscow&minStars=4"
```

```text
первый запрос -> X-Cache: MISS
второй запрос -> X-Cache: HIT
```

## Пункт 4. Проектирование rate limiting

Для обязательной реализации выбран endpoint:

```text
POST /bookings
```

Почему он требует rate limiting:

```text
создаёт бронирование
пишет в PostgreSQL
проверяет доступность номера
может вызвать конкуренцию за один номер
может быть целью автоматизированных запросов
```

Выбранный алгоритм:

```text
Token Bucket
```

Причина выбора:

```text
допускает короткие bursts
не мешает нормальному пользователю сделать несколько быстрых действий
ограничивает постоянный поток запросов
подходит для write endpoint'ов
```

Проектные лимиты:

```text
POST /bookings       10 запросов в минуту для обычного пользователя
POST /bookings       30 запросов в минуту для premium-пользователя
POST /auth/login      5 попыток в минуту
POST /auth/register  10 регистраций в минуту
GET /hotels         100 запросов в минуту
```

```text
POST /bookings -> 10 запросов в минуту на пользователя
```

## Пункт 5. Реализация rate limiting

Добавлен компонент:

```text
src/rate_limiter.hpp
src/rate_limiter.cpp
```

`RateLimiter` реализует Token Bucket.

Параметры:

```text
capacity = 10 токенов
refill_period = 60 секунд
стоимость запроса = 1 токен
```

Ключ ограничения:

```text
bookings:user:{userId}
```

Если лимит не превышен, запрос проходит дальше и создаёт бронирование.

Если лимит превышен, API возвращает:

```text
HTTP 429 Too Many Requests
```

Тело ответа:

```json
{
  "error": "rate_limit_exceeded",
  "message": "Too many booking requests. Please retry later."
}
```

Заголовки:

```text
X-RateLimit-Limit
X-RateLimit-Remaining
X-RateLimit-Reset
```

Пример проверки:

```bash
TOKEN="<TOKEN_FROM_LOGIN>"

for i in {1..12}; do
  curl -i -X POST http://localhost:8080/bookings \
    -H "Content-Type: application/json" \
    -H "Authorization: Bearer $TOKEN" \
    -d '{"roomId":1,"checkIn":"2026-12-10","checkOut":"2026-12-11","guestsCount":1}'
done
```

После исчерпания лимита должен вернуться:

```text
429 Too Many Requests
```

## Пункт 6. Анализ влияния на производительность

Кеширование улучшает производительность за счёт того, что повторные GET-запросы не обращаются к PostgreSQL.

Пример:

```text
100 одинаковых GET /hotels?city=Moscow
без кеша: 100 SQL-запросов
с кешем: 1 SQL-запрос + 99 cache hits
```

Rate limiting улучшает устойчивость системы за счёт ограничения дорогих write-операций.

Основные метрики мониторинга:

```text
http_request_duration_ms
http_429_total
cache_hits_total
cache_misses_total
cache_hit_rate
cache_invalidations_total
db_queries_total
db_query_duration_ms
rate_limit_allowed_total
rate_limit_rejected_total
```

Формула cache hit rate:

```text
cache_hit_rate = cache_hits / (cache_hits + cache_misses)
```

Пример:

```text
cache_hits = 90
cache_misses = 10

cache_hit_rate = 90 / (90 + 10) = 90%
```

Дополнительное описание находится в:

```text
performance_analysis.md
```

## Запуск проекта

### 1. Запуск через Docker Compose

```bash
docker compose up --build
```

Если нужно поднять проект с чистого состояния:

```bash
docker compose down -v
docker compose up --build
```

### 2. Проверка health endpoint

```bash
curl http://localhost:8080/ping
```

### 3. Проверка кеширования отелей

```bash
curl -i "http://localhost:8080/hotels?city=Moscow&minStars=4"
curl -i "http://localhost:8080/hotels?city=Moscow&minStars=4"
```

Ожидаемо:

```text
первый запрос -> X-Cache: MISS
второй запрос -> X-Cache: HIT
```

### 4. Проверка кеширования номеров

```bash
curl -i "http://localhost:8080/rooms?hotelId=1"
curl -i "http://localhost:8080/rooms?hotelId=1"
```

Ожидаемо:

```text
первый запрос -> X-Cache: MISS
второй запрос -> X-Cache: HIT
```

### 5. Проверка инвалидации кеша отелей

```bash
curl -i "http://localhost:8080/hotels?city=Moscow&minStars=4"

curl -i -X POST http://localhost:8080/hotels \
  -H "Content-Type: application/json" \
  -d '{"name":"Cache Test Hotel","city":"Moscow","address":"Cache Street 1","stars":4,"description":"Created for cache invalidation test"}'

curl -i "http://localhost:8080/hotels?city=Moscow&minStars=4"
```

После создания отеля следующий GET должен вернуть:

```text
X-Cache: MISS
```

### 6. Регистрация и авторизация

```bash
curl -X POST http://localhost:8080/auth/register \
  -H "Content-Type: application/json" \
  -d '{"firstName":"Roman","lastName":"Ivanov","login":"roman_lab5","email":"roman_lab5@example.com","password":"12345"}'
```

```bash
curl -X POST http://localhost:8080/auth/login \
  -H "Content-Type: application/json" \
  -d '{"login":"roman_lab5","password":"12345"}'
```

Из ответа нужно взять `token`.

### 7. Проверка rate limiting

```bash
TOKEN="<TOKEN_FROM_LOGIN>"

for i in {1..12}; do
  curl -i -X POST http://localhost:8080/bookings \
    -H "Content-Type: application/json" \
    -H "Authorization: Bearer $TOKEN" \
    -d '{"roomId":1,"checkIn":"2026-12-10","checkOut":"2026-12-11","guestsCount":1}'
done
```

Ожидаемые заголовки:

```text
X-RateLimit-Limit: 10
X-RateLimit-Remaining: ...
X-RateLimit-Reset: ...
```

После превышения лимита:

```text
HTTP/1.1 429 Too Many Requests
```

## Важные замечания по реализации

Для лабораторной работы используется in-memory cache и in-memory rate limiter. Это подходит для одного API-инстанса.

Для production-версии при горизонтальном масштабировании лучше вынести состояние в Redis:

```text
ResponseCache -> Redis
RateLimiter   -> Redis
```

Так все API-инстансы будут использовать общий кеш и общие bucket'и rate limiting.

## Итог

В рамках пятой лабораторной работы реализовано:

```text
анализ hot paths
стратегия кеширования
in-memory Cache-Aside для GET /hotels
in-memory Cache-Aside для GET /rooms
инвалидация кеша при POST /hotels, POST /bookings, DELETE /bookings
проектирование rate limiting
Token Bucket rate limiting для POST /bookings
HTTP 429 при превышении лимита
заголовки X-RateLimit-Limit, X-RateLimit-Remaining, X-RateLimit-Reset
анализ влияния оптимизаций на производительность
```
