cmake_minimum_required(VERSION 3.16)
project(hotel_booking_api)

set(CMAKE_CXX_STANDARD 17)

find_package(nlohmann_json REQUIRED)

include_directories(src)

add_executable(hotel-booking-api
    src/main.cpp
    src/handlers.cpp
    src/models.cpp
    src/auth.cpp
)

target_link_libraries(hotel-booking-api nlohmann_json::nlohmann_json)
