# Лабораторная работа №1

## Содержание

1. [Задание 1](#задание_1)
2. [Задание 2](#задание_2)
3. [Задание 3](#задание_3)
4. [Задание 4](#задание_4)
5. [Задание 5](#задание_5)
6. [Задание 6](#задание_6)
7. [Задание 7](#задание_7)
8. [Задание 8](#задание_8)
9. [Задание 9](#задание_9)

## задание_1

Изучите текст задания
-	Назначенный вариант из homeworks_variants.pdf
-	Изучите описание системы и требования к API
  
### Вариант №13

Система бронирования отелей https://www.booking.com/
Приложение должно содержать следующие данные:
-	Пользователь
-	Отель
-	Бронирование

Реализовать API:
-	Создание нового пользователя
-	Поиск пользователя по логину
-	Поиск пользователя по маске имя и фамилии
-	Создание отеля
-	Получение списка отелей
-	Поиск отелей по городу
-	Создание бронирования
-	Получение бронирований пользователя
-	Отмена бронирования


## задание_2

Определите перечень ролей пользователей и перечень внешних систем

-	Определите, какие роли пользователей будут взаимодействовать с системой
Клиент

-	Определите внешние системы, с которыми ваша система будет взаимодействовать (платежные системы, email-сервисы, SMS-сервисы и т.д.)

Billing Provider (Платежный провайдер)

Mail Provider (Почтовый провайдер)

## Задание 3

-	Опишите вашу систему как softwareSystem в Structurizr DSL


-	Используйте плагин к Visual Studio Code https://marketplace.visualstudio.com/items?itemName=vimpelcom.c4-varp
p.s. С visualstuido возникли проблемы, пользовался этим - https://playground.structurizr.com/
-	Создайте диаграмму System Context (C1), показывающую систему в контексте пользователей и внешних систем


<img width="2430" height="1050" alt="Context-dark" src="https://github.com/user-attachments/assets/6b2121bc-5098-4d87-b83c-95e8d62ee696" />


## задание_4

Продумайте основные задачи пользователей и как они могут быть реализованы
-	Определите основные use cases для каждой роли пользователя

Управление аккаунтом, Работа с отелями, управление бронированиями

-	Подумайте, какие контейнеры (веб-приложение, API, база данных и т.д.) необходимы для реализации этих задач

Client Application — веб или мобильное приложение для взаимодействия с пользователем.

Серверная часть (API и логика):
2. Backend Gateway — единая точка входа для всех API-запросов.
3. Identity Service — обработка пользователей (регистрация, поиск).
4. Hotel Catalog Service — управление отелями (добавление, поиск).
5. Reservation Service — управление бронированиями (создание, отмена, оплата).

Хранилища данных:
6. Main Database (PostgreSQL) — основное реляционное хранилище для всех сущностей.
7. Cache Layer (Redis) — кэширование данных для ускорения поиска.

Интеграция и уведомления:
8. Event Queue (Kafka) — асинхронная передача событий между сервисами.
9. Notification Processor — обработчик событий для отправки уведомлений.

## задание_5

Сформируйте перечень container, отвечающих за обработку событий, связанных с объектами предметной области
-	Определите контейнеры системы (например: Клиентский сервис, Сервис управления доставкой, Сервис регистрации платежей и т.д.)

-	Каждый контейнер должен отвечать за определенную функциональность из вашего варианта задания

## задание_6

Определите взаимодействие между контейнерами
-	Опишите, как контейнеры взаимодействуют друг с другом

Контейнеры взаимодействуют синхронно (через API) и асинхронно (через события):

**1. Синхронное взаимодействие (HTTP/REST API):**
Это основной способ обработки пользовательских запросов.
-   **Client App** -> **Gateway**: Все запросы от приложения поступают на единый шлюз.
-   **Gateway** -> **Identity/Catalog/Reservation Service**: Шлюз маршрутизирует запросы к соответствующему бизнес-сервису.
-   **Бизнес-сервисы** -> **Main Database**: Каждый сервис напрямую обращается к своей части схемы в PostgreSQL для CRUD операций.
-   **Бизнес-сервисы** -> **Cache Layer**: Сервисы проверяют наличие данных в Redis перед обращением к БД или обновляют кэш после записи.
-   **Reservation Service** -> **Billing Provider**: Сервис бронирования синхронно вызывает внешний платежный шлюз для проведения оплаты или возврата.

**2. Асинхронное взаимодействие (События через Kafka):**
Используется для слабосвязанных операций, не требующих немедленного ответа пользователю.
-   **Identity/Reservation Service** -> **Event Queue**: Сервисы публикуют события (регистрация, бронирование, отмена) в топики Kafka.
-   **Notification Processor** -> **Event Queue**: Воркер постоянно слушает очередь, читает новые события.
-   **Notification Processor** -> **Mail Provider**: Обработав событие, воркер синхронно вызывает внешний email-сервис для отправки уведомления.

-	Определите основные сценарии взаимодействия (создание пользователя, создание заказа на доставку и т.д.)

На основе диаграмм динамической взаимосвязи (dynamic views) определены следующие основные сценарии взаимодействия:

**Сценарии, инициируемые клиентом:**

1.  **Создание нового пользователя (RegisterUserFlow):**
    *Клиент -> Client App -> Gateway -> Identity Service -> Main Database -> Event Queue.*

2.  **Поиск пользователя по логину (SearchUserLogin):**
    *Клиент -> Client App -> Gateway -> Identity Service -> Cache Layer / Main Database.*

3.  **Поиск пользователя по маске имени (SearchUserByName):**
    *Клиент -> Client App -> Gateway -> Identity Service -> Main Database.*

4.  **Создание отеля (CreateHotelFlow):**
    *Клиент -> Client App -> Gateway -> Catalog Service -> Main Database.*

5.  **Получение списка отелей (ListHotelsFlow):**
    *Клиент -> Client App -> Gateway -> Catalog Service -> Cache Layer / Main Database.*

6.  **Поиск отелей по городу (FindHotelsCity):**
    *Клиент -> Client App -> Gateway -> Catalog Service -> Main Database.*

7.  **Создание бронирования (CreateReservation):**
    *Клиент -> Client App -> Gateway -> Reservation Service -> Billing Provider -> Main Database -> Event Queue.*

8.  **Получение бронирований пользователя (UserReservations):**
    *Клиент -> Client App -> Gateway -> Reservation Service -> Main Database.*

9.  **Отмена бронирования (CancelReservation):**
    *Клиент -> Client App -> Gateway -> Reservation Service -> Main Database -> Billing Provider -> Event Queue.*


## задание_7

Опишите модель container в Structurizr DSL и создайте диаграмму Container
-	Создайте описание всех контейнеров в Structurizr DSL
-	Создайте диаграмму Container (C2), показывающую контейнеры и их взаимодействие

  <img width="4110" height="2714" alt="Containers-dark (4)" src="https://github.com/user-attachments/assets/2e6d5af3-99d6-4e16-8b94-4d367cca6e77" />

## задание_8

Определите технологии и проставьте их на контейнерах и связях
-	Укажите технологии для каждого контейнера (например: "Web Application", "REST API", "PostgreSQL Database")

  **Клиентская часть:**
-   **Client Application:** React (для Web) или нативное мобильное приложение (Mobile App).

**API и бизнес-логика (микросервисы):**
-   **Backend Gateway:** Spring Boot (REST API)
-   **Identity Service:** Spring Boot (REST API)
-   **Hotel Catalog Service:** Spring Boot (REST API)
-   **Reservation Service:** Spring Boot (REST API)
-   **Notification Processor:** Worker (воркер, реализованный, предположительно, на Spring Boot или аналогичной платформе)

**Хранилища данных:**
-   **Main Database:** PostgreSQL (реляционная база данных)
-   **Cache Layer:** Redis (кэш)

**Инфраструктура и интеграция:**
-   **Event Queue:** Kafka (очередь событий)

  
Укажите протоколы взаимодействия на связях (например: "HTTPS/REST", "JDBC")

| От (контейнер)          | К (контейнер)            | Протокол взаимодействия            |
| ----------------------- | ------------------------ | ---------------------------------- |
| Client Application      | Backend Gateway          | **HTTPS / REST (JSON)**            |
| Backend Gateway         | Identity Service         | **HTTPS / REST (JSON)** (межсервисный) |
| Backend Gateway         | Catalog Service          | **HTTPS / REST (JSON)** (межсервисный) |
| Backend Gateway         | Reservation Service      | **HTTPS / REST (JSON)** (межсервисный) |
| Identity Service        | Main Database            | **JDBC / PostgreSQL Protocol**     |
| Catalog Service         | Main Database            | **JDBC / PostgreSQL Protocol**     |
| Reservation Service     | Main Database            | **JDBC / PostgreSQL Protocol**     |
| Identity Service        | Cache Layer (Redis)      | **Redis Protocol**                  |
| Catalog Service         | Cache Layer (Redis)      | **Redis Protocol**                  |
| Reservation Service     | Cache Layer (Redis)      | **Redis Protocol**                  |
| Reservation Service     | Billing Provider (внешн.) | **HTTPS / REST (JSON)** (внешний API) |
| Reservation Service     | Event Queue (Kafka)      | **Kafka Protocol** (публикация)    |
| Identity Service        | Event Queue (Kafka)      | **Kafka Protocol** (публикация)    |
| Notification Processor  | Event Queue (Kafka)      | **Kafka Protocol** (потребление)   |
| Notification Processor  | Mail Provider (внешн.)   | **HTTPS / REST (JSON)** (или SMTP) |


## задание_9

9.	Создайте одну диаграмму dynamic для архитектурно значимого варианта использования
-	Выберите один важный сценарий из вашего варианта (отправка сообщения между пользователями, покупка товара в магазине, создание бронирования и т.д.)
-	Создайте диаграмму Dynamic, показывающую последовательность взаимодействия между контейнерами при выполнении этого сценария

Пример - создание нового пользователя  
<img width="4110" height="1229" alt="RegisterUserFlow-dark" src="https://github.com/user-attachments/assets/20493e6c-854e-4134-9e26-b5b9977277af" />




