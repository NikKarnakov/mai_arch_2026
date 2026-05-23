cmake_minimum_required(VERSION 3.16)
project(hotel_booking_api CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(userver REQUIRED COMPONENTS core postgresql mongo)

add_executable(hotel-booking-api
    src/main.cpp
    src/auth.cpp
    src/storage.cpp
    src/handlers.cpp
    src/response_cache.cpp
    src/rate_limiter.cpp
    src/mongo_storage.cpp
    src/mongo_handlers.cpp
)

target_link_libraries(hotel-booking-api
    userver::core
    userver::postgresql
    userver::mongo
)

target_include_directories(hotel-booking-api PRIVATE src)

userver_testsuite_add_simple()
