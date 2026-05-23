# Лабораторная работа №4

Этот вариант объединяет PostgreSQL-часть третьей лабораторной работы и MongoDB-часть четвертой лабораторной работы. 

## Что есть в проекте

- PostgreSQL-схема: `schema.sql`, `data.sql`, `queries.sql`, `optimization.md`;
- MongoDB-документы: `schema_design.md`, `data.js`, `queries.js`, `validation.js`, `aggregation.js`;
- API на userver для PostgreSQL: пользователи, отели, номера, бронирования, отзывы;
- API на userver для MongoDB: `/mongo/bookings`, `/mongo/reviews`;
- Dockerfile и docker-compose для запуска API + PostgreSQL + MongoDB.

## Запуск

```bash
docker compose down -v
docker compose up --build
```

Проверка:

```bash
curl http://localhost:8080/ping
```

## Основные endpoint'ы PostgreSQL API

```text
POST   /auth/register
POST   /auth/login
GET    /users?login=ivanp
GET    /users?mask=pet
POST   /hotels
GET    /hotels
GET    /hotels?city=Moscow&minStars=4
GET    /rooms?hotelId=1
GET    /rooms?hotelId=1&roomType=standard&guests=2&checkIn=2026-06-10&checkOut=2026-06-12
POST   /bookings
GET    /bookings
DELETE /bookings
POST   /reviews
```

## Основные endpoint'ы MongoDB API

```text
GET    /mongo/bookings?bookingId=BK-2026-0001
GET    /mongo/bookings?userId=USR-0001
POST   /mongo/bookings
DELETE /mongo/bookings
GET    /mongo/reviews?hotelId=HTL-0002
POST   /mongo/reviews
```

## Проверка MongoDB-скриптов


```bash
mongosh "mongodb://localhost:27017/hotel_booking_docs" data.js
mongosh "mongodb://localhost:27017/hotel_booking_docs" queries.js
mongosh "mongodb://localhost:27017/hotel_booking_docs" validation.js
mongosh "mongodb://localhost:27017/hotel_booking_docs" aggregation.js
```
