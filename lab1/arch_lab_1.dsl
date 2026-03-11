workspace "Hotel Reservation Platform" "Система бронирования отелей" {

    model {

        client = person "Клиент" "Ищет отели и оформляет бронирования."

        bookingPlatform = softwareSystem "Hotel Reservation Platform" "Платформа для управления отелями и бронированиями." {

            clientApp = container "Client Application" "Web или мобильное приложение пользователя." "React / Mobile App"

            gateway = container "Backend Gateway" "Основная точка входа для клиентских запросов." "Spring Boot"

            identityService = container "Identity Service" "Управление пользователями и их поиск." "Spring Boot"

            catalogService = container "Hotel Catalog Service" "Управление данными об отелях и поиск по городу." "Spring Boot"

            reservationService = container "Reservation Service" "Создание и управление бронированиями." "Spring Boot"

            eventQueue = container "Event Queue" "Передача событий между сервисами." "Kafka"

            notificationProcessor = container "Notification Processor" "Обрабатывает события и уведомляет пользователей." "Worker"

            cacheLayer = container "Cache Layer" "Кэш часто используемых данных." "Redis"

            mainDatabase = container "Main Database" "Основная база данных системы." "PostgreSQL"

        }

        mailProvider = softwareSystem "Mail Provider" "Сервис отправки email уведомлений"
        billingProvider = softwareSystem "Billing Provider" "Сервис обработки платежей"


        client -> clientApp "Использует приложение"
        clientApp -> gateway "Отправляет API запросы"

        gateway -> identityService "Операции с пользователями"
        gateway -> catalogService "Операции с отелями"
        gateway -> reservationService "Операции с бронированием"

        identityService -> mainDatabase "Работа с пользователями"
        catalogService -> mainDatabase "Работа с отелями"
        reservationService -> mainDatabase "Работа с бронированиями"

        identityService -> cacheLayer "Кэш пользователей"
        catalogService -> cacheLayer "Кэш отелей"
        reservationService -> cacheLayer "Кэш бронирований"

        reservationService -> billingProvider "Оплата бронирования"

        reservationService -> eventQueue "Публикует события бронирования"
        identityService -> eventQueue "Публикует события регистрации"

        notificationProcessor -> eventQueue "Читает события"
        notificationProcessor -> mailProvider "Отправляет уведомления"

    }

    views {

        systemContext bookingPlatform "Context" {
            include *
            autolayout lr
        }

        container bookingPlatform "Containers" {
            include *
            autolayout lr
        }


        dynamic bookingPlatform "RegisterUserFlow" {
            title "Создание нового пользователя"
            autolayout lr

            client -> clientApp "1. Регистрация"
            clientApp -> gateway "2. POST /users"
            gateway -> identityService "3. Create account"
            identityService -> mainDatabase "4. Save data"
            identityService -> eventQueue "5. Publish registration event"
            gateway -> clientApp "6. Response"
        }


        dynamic bookingPlatform "SearchUserLogin" {
            title "Поиск пользователя по логину"
            autolayout lr

            client -> clientApp "1. Ввод логина"
            clientApp -> gateway "2. GET /users/login"
            gateway -> identityService "3. Search user"
            identityService -> cacheLayer "4. Check cache"
            identityService -> mainDatabase "5. Load user"
            gateway -> clientApp "6. Return user"
        }


        dynamic bookingPlatform "SearchUserByName" {
            title "Поиск пользователя по маске имени"
            autolayout lr

            client -> clientApp "1. Ввод имени"
            clientApp -> gateway "2. GET /users/search"
            gateway -> identityService "3. Find users"
            identityService -> mainDatabase "4. Query database"
            gateway -> clientApp "5. Return result"
        }


        dynamic bookingPlatform "CreateHotelFlow" {
            title "Добавление нового отеля"
            autolayout lr

            client -> clientApp "1. Добавить отель"
            clientApp -> gateway "2. POST /hotels"
            gateway -> catalogService "3. Save hotel"
            catalogService -> mainDatabase "4. Insert hotel"
            gateway -> clientApp "5. Success response"
        }


        dynamic bookingPlatform "ListHotelsFlow" {
            title "Получение списка отелей"
            autolayout lr

            client -> clientApp "1. Запрос списка"
            clientApp -> gateway "2. GET /hotels"
            gateway -> catalogService "3. Load hotels"
            catalogService -> cacheLayer "4. Check cache"
            catalogService -> mainDatabase "5. Query DB"
            gateway -> clientApp "6. Return hotels"
        }


        dynamic bookingPlatform "FindHotelsCity" {
            title "Поиск отелей по городу"
            autolayout lr

            client -> clientApp "1. Ввод города"
            clientApp -> gateway "2. GET /hotels/city"
            gateway -> catalogService "3. Search hotels"
            catalogService -> mainDatabase "4. Query by city"
            gateway -> clientApp "5. Return hotels"
        }


        dynamic bookingPlatform "CreateReservation" {
            title "Создание бронирования"
            autolayout lr

            client -> clientApp "1. Бронирование номера"
            clientApp -> gateway "2. POST /reservations"
            gateway -> reservationService "3. Create reservation"
            reservationService -> billingProvider "4. Process payment"
            reservationService -> mainDatabase "5. Save reservation"
            reservationService -> eventQueue "6. Publish event"
            gateway -> clientApp "7. Response"
        }


        dynamic bookingPlatform "UserReservations" {
            title "Получение бронирований пользователя"
            autolayout lr

            client -> clientApp "1. Просмотр бронирований"
            clientApp -> gateway "2. GET /users/{id}/reservations"
            gateway -> reservationService "3. Fetch reservations"
            reservationService -> mainDatabase "4. Query reservations"
            gateway -> clientApp "5. Return list"
        }


        dynamic bookingPlatform "CancelReservation" {
            title "Отмена бронирования"
            autolayout lr

            client -> clientApp "1. Отмена бронирования"
            clientApp -> gateway "2. DELETE /reservations/{id}"
            gateway -> reservationService "3. Cancel reservation"
            reservationService -> mainDatabase "4. Update status"
            reservationService -> billingProvider "5. Refund payment"
            reservationService -> eventQueue "6. Publish cancellation"
            gateway -> clientApp "7. Confirmation"
        }

    }

}
