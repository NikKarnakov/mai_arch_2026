#include "models.hpp"
#include <algorithm>
#include <stdexcept>

UserStore& UserStore::Instance() {
    static UserStore instance;
    return instance;
}

int UserStore::AddUser(const nlohmann::json& data) {
    for (const auto& u : users_) {
        if (u.login == data["login"]) throw std::runtime_error("Login exists");
    }
    User user{nextId_++, data["firstName"], data["lastName"], data["login"], data["email"], data["password"]};
    users_.push_back(user);
    return user.id;
}

User* UserStore::FindByLogin(const std::string& login) {
    auto it = std::find_if(users_.begin(), users_.end(),
                           [&](const User& u){ return u.login == login; });
    return it != users_.end() ? &(*it) : nullptr;
}

std::vector<User> UserStore::SearchByNameMask(const std::string& mask) {
    std::vector<User> result;
    for (auto& u : users_) {
        if (u.firstName.find(mask) != std::string::npos ||
            u.lastName.find(mask) != std::string::npos) {
            result.push_back(u);
        }
    }
    return result;
}

HotelStore& HotelStore::Instance() {
    static HotelStore instance;
    return instance;
}

int HotelStore::AddHotel(const nlohmann::json& data) {
    Hotel h{nextId_++, data["name"], data["city"]};
    hotels_.push_back(h);
    return h.id;
}

nlohmann::json HotelStore::ListHotels() {
    nlohmann::json arr = nlohmann::json::array();
    for (auto& h : hotels_) arr.push_back({{"id", h.id}, {"name", h.name}, {"city", h.city}});
    return arr;
}


BookingStore& BookingStore::Instance() {
    static BookingStore instance;
    return instance;
}

int BookingStore::CreateBooking(const nlohmann::json& data, const std::string& login) {
    Booking b{nextId_++, data["hotelId"], login};
    bookings_.push_back(b);
    return b.id;
}

bool BookingStore::CancelBooking(int bookingId, const std::string& login) {
    auto it = std::find_if(bookings_.begin(), bookings_.end(),
                           [&](const Booking& b){ return b.id == bookingId && b.userLogin == login; });
    if (it != bookings_.end()) {
        bookings_.erase(it);
        return true;
    }
    return false;
}
