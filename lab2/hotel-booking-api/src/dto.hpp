#pragma once
#include <string>

struct UserRegisterDTO {
    std::string firstName;
    std::string lastName;
    std::string login;
    std::string email;
    std::string password;
};

struct UserLoginDTO {
    std::string login;
    std::string password;
};

struct HotelDTO {
    std::string name;
    std::string city;
};

struct BookingDTO {
    int hotelId;
};
