cmake_minimum_required(VERSION 3.16)
project(hotel_booking_mongo_api CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(userver REQUIRED COMPONENTS core mongo)

add_executable(hotel-booking-mongo-api
    src/main.cpp
    src/mongo_storage.cpp
    src/handlers.cpp
)

target_link_libraries(hotel-booking-mongo-api
    userver::core
    userver::mongo
)

target_include_directories(hotel-booking-mongo-api PRIVATE src)
