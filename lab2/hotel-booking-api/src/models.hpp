#pragma once
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct User {
    int id;
    std::string firstName;
    std::string lastName;
    std::string login;
    std::string email;
    std::string password;
};

struct Hotel {
    int id;
    std::string name;
    std::string city;
};

struct Booking {
    int id;
    int hotelId;
    std::string userLogin;
};

class UserStore {
public:
    static UserStore& Instance();
    int AddUser(const nlohmann::json& data);
    User* FindByLogin(const std::string& login);
    std::vector<User> SearchByNameMask(const std::string& mask);
private:
    std::vector<User> users_;
    int nextId_ = 1;
};

class HotelStore {
public:
    static HotelStore& Instance();
    int AddHotel(const nlohmann::json& data);
    nlohmann::json ListHotels();
private:
    std::vector<Hotel> hotels_;
    int nextId_ = 1;
};

class BookingStore {
public:
    static BookingStore& Instance();
    int CreateBooking(const nlohmann::json& data, const std::string& login);
    bool CancelBooking(int bookingId, const std::string& login);
private:
    std::vector<Booking> bookings_;
    int nextId_ = 1;
};
