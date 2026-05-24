#include "auth.hpp"

#include <charconv>
#include <functional>
#include <random>
#include <sstream>
#include <string_view>

namespace hotel::auth {

std::string HashPassword(const std::string& password) {
    const auto value = std::hash<std::string>{}("hotel-booking:" + password);
    return "demo$" + std::to_string(value);
}

bool CheckPassword(const std::string& password, const std::string& password_hash) {
    return HashPassword(password) == password_hash;
}

std::string MakeToken(long user_id, const std::string& login) {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    const auto nonce = gen();

    std::ostringstream out;
    out << user_id << ':' << login << ':' << nonce;
    return out.str();
}

std::optional<long> ExtractUserId(const std::string& authorization_header) {
    constexpr std::string_view prefix = "Bearer ";

    if (authorization_header.rfind(std::string(prefix), 0) != 0) {
        return std::nullopt;
    }

    const auto token = authorization_header.substr(prefix.size());
    const auto sep = token.find(':');
    if (sep == std::string::npos) {
        return std::nullopt;
    }

    long user_id = 0;
    const auto* begin = token.data();
    const auto* end = token.data() + sep;

    const auto [ptr, ec] = std::from_chars(begin, end, user_id);
    if (ec != std::errc{} || ptr != end || user_id <= 0) {
        return std::nullopt;
    }

    return user_id;
}

} 
