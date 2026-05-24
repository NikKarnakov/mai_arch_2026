#include "handlers.hpp"

#include "auth.hpp"

#include <stdexcept>
#include <chrono>

#include <userver/formats/json/value_builder.hpp>
#include <userver/formats/json/serialize.hpp>
#include <userver/server/http/http_status.hpp>

namespace hotel {

namespace {


constexpr auto kCatalogCacheTtl = std::chrono::seconds{60};

constexpr int kBookingRateLimit = 10;
constexpr auto kBookingRatePeriod = std::chrono::seconds{60};

std::string BookingRateLimitKey(long user_id) {
    return "bookings:user:" + std::to_string(user_id);
}

void SetRateLimitHeaders(userver::server::http::HttpResponse& response,
                         const RateLimiter::Decision& decision) {
    response.SetHeader("X-RateLimit-Limit", std::to_string(decision.limit));
    response.SetHeader("X-RateLimit-Remaining", std::to_string(decision.remaining));
    response.SetHeader("X-RateLimit-Reset", std::to_string(decision.reset_after_seconds));
}


std::string HotelsCacheKey(const userver::server::http::HttpRequest& request) {
    const auto city = request.GetArg("city");
    const auto min_stars = request.GetArg("minStars");

    return "hotels:city:" + (city.empty() ? std::string{"all"} : city) +
           ":min_stars:" + (min_stars.empty() ? std::string{"1"} : min_stars);
}

std::string RoomsCacheKey(const userver::server::http::HttpRequest& request) {
    return "rooms:hotel:" + request.GetArg("hotelId");
}

userver::formats::json::Value JsonFromCache(std::string_view cached) {
    return userver::formats::json::FromString(std::string{cached});
}

std::string JsonToCache(const userver::formats::json::Value& value) {
    return userver::formats::json::ToString(value);
}

std::string GetString(const userver::formats::json::Value& body, std::string_view name) {
    if (!body.HasMember(std::string{name})) {
        throw std::runtime_error("Missing field: " + std::string{name});
    }
    return body[std::string{name}].As<std::string>();
}

int GetInt(const userver::formats::json::Value& body, std::string_view name, int fallback = 0) {
    if (!body.HasMember(std::string{name})) {
        return fallback;
    }
    return body[std::string{name}].As<int>();
}

long GetLong(const userver::formats::json::Value& body, std::string_view name) {
    if (!body.HasMember(std::string{name})) {
        throw std::runtime_error("Missing field: " + std::string{name});
    }
    return body[std::string{name}].As<long>();
}

void PutRoom(userver::formats::json::ValueBuilder& item, const RoomRow& room) {
    item["id"] = room.id;
    item["hotelId"] = room.hotel_id;
    item["roomNumber"] = room.room_number;
    item["roomType"] = room.room_type;
    item["capacity"] = room.capacity;
    item["pricePerNight"] = room.price_per_night;
    item["isAvailable"] = room.is_available;
}

long RequireUserId(const userver::server::http::HttpRequest& request) {
    const auto user_id = auth::ExtractUserId(request.GetHeader("Authorization"));
    if (!user_id) {
        throw std::runtime_error("Unauthorized");
    }
    return *user_id;
}

} 

BaseJsonHandler::BaseJsonHandler(const userver::components::ComponentConfig& config,
                                 const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context),
      storage_(context.FindComponent<Storage>()),
      cache_(context.FindComponent<ResponseCache>()),
      rate_limiter_(context.FindComponent<RateLimiter>()) {}

const Storage& BaseJsonHandler::StorageRef() const {
    return storage_;
}

ResponseCache& BaseJsonHandler::CacheRef() const {
    return cache_;
}

RateLimiter& BaseJsonHandler::RateLimiterRef() const {
    return rate_limiter_;
}

userver::formats::json::Value RegisterHandler::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value& body,
    userver::server::request::RequestContext&) const {
    auto& response = request.GetHttpResponse();

    const auto id = StorageRef().CreateUser(
        GetString(body, "firstName"),
        GetString(body, "lastName"),
        GetString(body, "login"),
        GetString(body, "email"),
        auth::HashPassword(GetString(body, "password")));

    response.SetStatus(userver::server::http::HttpStatus::kCreated);

    userver::formats::json::ValueBuilder result;
    result["id"] = id;
    return result.ExtractValue();
}

userver::formats::json::Value LoginHandler::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value& body,
    userver::server::request::RequestContext&) const {
    const auto login = GetString(body, "login");
    const auto password = GetString(body, "password");

    const auto user = StorageRef().GetUserByLogin(login);
    if (!user || !auth::CheckPassword(password, user->password_hash)) {
        request.GetHttpResponse().SetStatus(userver::server::http::HttpStatus::kUnauthorized);
        userver::formats::json::ValueBuilder err;
        err["error"] = "Invalid credentials";
        return err.ExtractValue();
    }

    userver::formats::json::ValueBuilder result;
    result["token"] = auth::MakeToken(user->id, user->login);
    result["userId"] = user->id;
    return result.ExtractValue();
}

userver::formats::json::Value UsersHandler::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value&,
    userver::server::request::RequestContext&) const {
    const auto login = request.GetArg("login");

    userver::formats::json::ValueBuilder result;

    if (!login.empty()) {
        const auto user = StorageRef().GetUserByLogin(login);
        if (!user) {
            request.GetHttpResponse().SetStatus(userver::server::http::HttpStatus::kNotFound);
            result["error"] = "User not found";
            return result.ExtractValue();
        }

        result["id"] = user->id;
        result["firstName"] = user->first_name;
        result["lastName"] = user->last_name;
        result["login"] = user->login;
        result["email"] = user->email;
        return result.ExtractValue();
    }

    const auto mask = request.GetArg("mask");
    if (mask.empty()) {
        request.GetHttpResponse().SetStatus(userver::server::http::HttpStatus::kBadRequest);
        result["error"] = "Use login or mask query parameter";
        return result.ExtractValue();
    }

    const auto users = StorageRef().SearchUsersByNameMask(mask);
    auto arr = userver::formats::json::ValueBuilder(userver::formats::json::Type::kArray);

    for (const auto& user : users) {
        userver::formats::json::ValueBuilder item;
        item["id"] = user.id;
        item["firstName"] = user.first_name;
        item["lastName"] = user.last_name;
        item["login"] = user.login;
        item["email"] = user.email;
        arr.PushBack(item.ExtractValue());
    }

    result["users"] = arr.ExtractValue();
    return result.ExtractValue();
}

userver::formats::json::Value HotelsHandler::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value& body,
    userver::server::request::RequestContext&) const {
    const auto method = request.GetMethod();

    if (method == userver::server::http::HttpMethod::kPost) {
        const auto id = StorageRef().CreateHotel(
            GetString(body, "name"),
            GetString(body, "city"),
            GetString(body, "address"),
            GetInt(body, "stars", 3),
            body.HasMember("description") ? body["description"].As<std::string>() : "");

        CacheRef().InvalidatePrefix("hotels:");

        request.GetHttpResponse().SetStatus(userver::server::http::HttpStatus::kCreated);
        request.GetHttpResponse().SetHeader("X-Cache-Invalidated", "hotels");

        userver::formats::json::ValueBuilder result;
        result["hotelId"] = id;
        return result.ExtractValue();
    }

    const auto cache_key = HotelsCacheKey(request);
    if (const auto cached = CacheRef().Get(cache_key)) {
        request.GetHttpResponse().SetHeader("X-Cache", "HIT");
        return JsonFromCache(*cached);
    }

    const auto city = request.GetArg("city");
    const auto stars_arg = request.GetArg("minStars");
    const auto min_stars = stars_arg.empty() ? 1 : std::stoi(stars_arg);

    const auto hotels = StorageRef().SearchHotels(city, min_stars);

    userver::formats::json::ValueBuilder result;
    auto arr = userver::formats::json::ValueBuilder(userver::formats::json::Type::kArray);

    for (const auto& hotel : hotels) {
        userver::formats::json::ValueBuilder item;
        item["id"] = hotel.id;
        item["name"] = hotel.name;
        item["city"] = hotel.city;
        item["address"] = hotel.address;
        item["stars"] = hotel.stars;
        item["description"] = hotel.description;
        arr.PushBack(item.ExtractValue());
    }

    result["hotels"] = arr.ExtractValue();
    result["cacheTtlSeconds"] = 60;

    auto value = result.ExtractValue();
    CacheRef().Put(cache_key, JsonToCache(value), kCatalogCacheTtl);
    request.GetHttpResponse().SetHeader("X-Cache", "MISS");
    return value;
}

userver::formats::json::Value RoomsHandler::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value&,
    userver::server::request::RequestContext&) const {
    const auto hotel_id_arg = request.GetArg("hotelId");
    if (hotel_id_arg.empty()) {
        throw std::runtime_error("hotelId query parameter is required");
    }

    const auto hotel_id = std::stol(hotel_id_arg);
    const auto room_type = request.GetArg("roomType");
    const auto check_in = request.GetArg("checkIn");
    const auto check_out = request.GetArg("checkOut");
    const auto guests_arg = request.GetArg("guests");
    const auto guests = guests_arg.empty() ? 1 : std::stoi(guests_arg);

    const bool availability_query = !check_in.empty() && !check_out.empty();

    if (!availability_query) {
        const auto cache_key = RoomsCacheKey(request);
        if (const auto cached = CacheRef().Get(cache_key)) {
            request.GetHttpResponse().SetHeader("X-Cache", "HIT");
            return JsonFromCache(*cached);
        }

        const auto rooms = StorageRef().ListRooms(hotel_id);

        userver::formats::json::ValueBuilder result;
        auto arr = userver::formats::json::ValueBuilder(userver::formats::json::Type::kArray);

        for (const auto& room : rooms) {
            userver::formats::json::ValueBuilder item;
            PutRoom(item, room);
            arr.PushBack(item.ExtractValue());
        }

        result["rooms"] = arr.ExtractValue();
        result["cacheTtlSeconds"] = 60;

        auto value = result.ExtractValue();
        CacheRef().Put(cache_key, JsonToCache(value), kCatalogCacheTtl);
        request.GetHttpResponse().SetHeader("X-Cache", "MISS");
        return value;
    }

    const auto rooms = StorageRef().FindAvailableRooms(hotel_id, room_type, guests, check_in, check_out);

    userver::formats::json::ValueBuilder result;
    auto arr = userver::formats::json::ValueBuilder(userver::formats::json::Type::kArray);

    for (const auto& room : rooms) {
        userver::formats::json::ValueBuilder item;
        PutRoom(item, room);
        arr.PushBack(item.ExtractValue());
    }

    result["rooms"] = arr.ExtractValue();
    result["cache"] = "disabled_for_availability_query";
    return result.ExtractValue();
}

userver::formats::json::Value BookingsHandler::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value& body,
    userver::server::request::RequestContext&) const {
    long user_id = 0;

    try {
        user_id = RequireUserId(request);
    } catch (const std::exception&) {
        request.GetHttpResponse().SetStatus(userver::server::http::HttpStatus::kUnauthorized);
        userver::formats::json::ValueBuilder err;
        err["error"] = "Unauthorized";
        return err.ExtractValue();
    }

    const auto method = request.GetMethod();

    if (method == userver::server::http::HttpMethod::kPost) {
        auto& response = request.GetHttpResponse();

        const auto rate_decision = RateLimiterRef().Consume(
            BookingRateLimitKey(user_id),
            kBookingRateLimit,
            kBookingRatePeriod
        );

        SetRateLimitHeaders(response, rate_decision);

        if (!rate_decision.allowed) {
            response.SetStatus(userver::server::http::HttpStatus::kTooManyRequests);

            userver::formats::json::ValueBuilder err;
            err["error"] = "rate_limit_exceeded";
            err["message"] = "Too many booking requests. Please retry later.";
            return err.ExtractValue();
        }

        const auto id = StorageRef().CreateBooking(
            user_id,
            GetLong(body, "roomId"),
            GetString(body, "checkIn"),
            GetString(body, "checkOut"),
            GetInt(body, "guestsCount", 1));

        request.GetHttpResponse().SetStatus(userver::server::http::HttpStatus::kCreated);
        CacheRef().InvalidatePrefix("rooms:");
        request.GetHttpResponse().SetHeader("X-Cache-Invalidated", "rooms");

        userver::formats::json::ValueBuilder result;
        result["bookingId"] = id;
        return result.ExtractValue();
    }

    if (method == userver::server::http::HttpMethod::kDelete) {
        const auto ok = StorageRef().CancelBooking(GetLong(body, "bookingId"), user_id);
        request.GetHttpResponse().SetStatus(
            ok ? userver::server::http::HttpStatus::kOk
               : userver::server::http::HttpStatus::kNotFound);
        if (ok) {
            CacheRef().InvalidatePrefix("rooms:");
            request.GetHttpResponse().SetHeader("X-Cache-Invalidated", "rooms");
        }

        userver::formats::json::ValueBuilder result;
        result["status"] = ok ? "cancelled" : "not_found";
        return result.ExtractValue();
    }

    const auto bookings = StorageRef().ListUserBookings(user_id);
    userver::formats::json::ValueBuilder result;
    auto arr = userver::formats::json::ValueBuilder(userver::formats::json::Type::kArray);

    for (const auto& booking : bookings) {
        userver::formats::json::ValueBuilder item;
        item["id"] = booking.id;
        item["userId"] = booking.user_id;
        item["roomId"] = booking.room_id;
        item["checkIn"] = booking.check_in;
        item["checkOut"] = booking.check_out;
        item["guestsCount"] = booking.guests_count;
        item["status"] = booking.status;
        item["totalPrice"] = booking.total_price;
        arr.PushBack(item.ExtractValue());
    }

    result["bookings"] = arr.ExtractValue();
    return result.ExtractValue();
}

userver::formats::json::Value ReviewsHandler::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value& body,
    userver::server::request::RequestContext&) const {
    long user_id = 0;

    try {
        user_id = RequireUserId(request);
    } catch (const std::exception&) {
        request.GetHttpResponse().SetStatus(userver::server::http::HttpStatus::kUnauthorized);
        userver::formats::json::ValueBuilder err;
        err["error"] = "Unauthorized";
        return err.ExtractValue();
    }

    std::optional<long> booking_id;
    if (body.HasMember("bookingId") && !body["bookingId"].IsNull()) {
        booking_id = body["bookingId"].As<long>();
    }

    const auto id = StorageRef().CreateReview(
        user_id,
        GetLong(body, "hotelId"),
        booking_id,
        GetInt(body, "rating", 5),
        body.HasMember("comment") ? body["comment"].As<std::string>() : "");

    request.GetHttpResponse().SetStatus(userver::server::http::HttpStatus::kCreated);

    userver::formats::json::ValueBuilder result;
    result["reviewId"] = id;
    return result.ExtractValue();
}

}  
