# Валидация схем MongoDB

Файл:

```text
validation.js
```

Запуск:

```bash
mongosh "mongodb://localhost:27017/hotel_booking_docs" validation.js
```

## Что делает скрипт

Скрипт настраивает `$jsonSchema`-валидацию для коллекции `bookings`.

Используется команда:

```javascript
db.runCommand({
  collMod: "bookings",
  validator: {
    $jsonSchema: { ... }
  },
  validationLevel: "strict",
  validationAction: "error"
})
```

## Проверяемые поля

Валидация задаёт обязательные поля:

```text
bookingId
userId
hotelId
roomId
status
period
guests
hotelSnapshot
roomSnapshot
price
payment
events
createdAt
updatedAt
```

## Проверяемые типы

Используются MongoDB/BSON-типы:

```text
object
string
date
int
double
decimal
array
```

## Дополнительные ограничения

Валидация проверяет:

- формат `bookingId` по pattern `^BK-[0-9]{4}-[0-9]{4}$`;
- формат `userId` по pattern `^USR-[0-9]{4}$`;
- формат `hotelId` по pattern `^HTL-[0-9]{4}$`;
- формат `roomId` по pattern `^ROOM-[0-9]{4}$`;
- допустимые статусы бронирования;
- количество ночей от 1 до 60;
- количество взрослых гостей от 1 до 10;
- количество детей от 0 до 10;
- звёзды отеля от 1 до 5;
- тип номера из списка `standard`, `comfort`, `family`, `luxury`;
- валюту из списка `RUB`, `USD`, `EUR`;
- статус оплаты из списка `pending`, `paid`, `refunded`, `failed`.

## Тестирование невалидных данных

В конце `validation.js` есть несколько попыток вставить неправильные документы:

1. неправильный формат `bookingId`;
2. недопустимый статус `archived`;
3. слишком большое количество ночей;
4. отсутствие обязательного объекта `price`;
5. некорректное количество звёзд отеля.

Каждая такая вставка должна завершиться ошибкой валидации.
