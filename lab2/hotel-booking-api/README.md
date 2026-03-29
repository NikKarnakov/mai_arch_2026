# Лабораторная работа №2
## Разработка REST API сервиса

## Вариант 13 Система бронирования отелей https://www.booking.com/
Приложение должно содержать следующие данные:
- Пользователь
- Отель
- Бронирование
Реализовать API:
- Создание нового пользователя
- Поиск пользователя по логину
- Поиск пользователя по маске имя и фамилии
- Создание отеля
- Получение списка отелей
- Поиск отелей по городу
- Создание бронирования
- Получение бронирований пользователя
- Отмена бронирования

## Запуск

### Docker
```bash
docker-compose up --build
```

### Локально

```bash
mkdir build
cd build
cmake ..
make
./hotel-booking-api
```

### Endpoints

- POST /auth/register - регистрация пользователя
- POST /auth/login - вход пользователя
- GET /hotels - список отелей
- POST /hotels - создать отель
- POST /bookings - создать бронирование
- DELETE /bookings - отменить бронирование

### Примеры запросов 

```bash
curl -X POST http://localhost:8080/auth/register \
  -H "Content-Type: application/json" \
  -d '{"firstName":"Ivan","lastName":"Petrov","login":"ivanp","email":"ivan@example.com","password":"12345"}'
```
p.s не судите строго
