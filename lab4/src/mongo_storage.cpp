#include "mongo_storage.hpp"

#include <userver/components/component_context.hpp>
#include <userver/formats/bson.hpp>
#include <userver/formats/bson/inline.hpp>
#include <userver/formats/json/value_builder.hpp>
#include <userver/storages/mongo/component.hpp>
#include <userver/storages/mongo/options.hpp>

namespace hotel {

namespace bson = userver::formats::bson;
namespace mongo = userver::storages::mongo;

namespace {

std::string GetString(const userver::formats::json::Value& body, std::string_view field) {
    return body[std::string{field}].As<std::string>();
}

int GetInt(const userver::formats::json::Value& body, std::string_view field, int fallback = 0) {
    if (!body.HasMember(std::string{field})) return fallback;
    return body[std::string{field}].As<int>();
}

double GetDouble(const userver::formats::json::Value& body, std::string_view field, double fallback = 0.0) {
    if (!body.HasMember(std::string{field})) return fallback;
    return body[std::string{field}].As<double>();
}

userver::formats::json::Value ToJson(const bson::Document& doc) {
    return userver::formats::json::FromString(bson::ToCanonicalJsonString(doc));
}

} 

MongoStorage::MongoStorage(const userver::components::ComponentConfig& config,
                           const userver::components::ComponentContext& context)
    : ComponentBase(config, context),
      pool_(context.FindComponent<userver::components::Mongo>("mongo-hotel-booking").GetPool()) {}

userver::formats::json::Value MongoStorage::CreateBooking(const userver::formats::json::Value& body) const {
    const auto booking_id = GetString(body, "bookingId");
    const auto user_id = GetString(body, "userId");
    const auto hotel_id = GetString(body, "hotelId");
    const auto room_id = GetString(body, "roomId");

    auto bookings = pool_->GetCollection("bookings");

    const auto document = bson::MakeDoc(
        "bookingId", booking_id,
        "userId", user_id,
        "hotelId", hotel_id,
        "roomId", room_id,
        "status", "created",
        "period", bson::MakeDoc(
            "checkIn", GetString(body, "checkIn"),
            "checkOut", GetString(body, "checkOut"),
            "nights", GetInt(body, "nights", 1)
        ),
        "guests", bson::MakeDoc(
            "adults", GetInt(body, "adults", 1),
            "children", GetInt(body, "children", 0),
            "guestNames", bson::MakeArray(GetString(body, "guestName"))
        ),
        "hotelSnapshot", bson::MakeDoc(
            "name", GetString(body, "hotelName"),
            "city", GetString(body, "city"),
            "address", GetString(body, "address"),
            "stars", GetInt(body, "stars", 3)
        ),
        "roomSnapshot", bson::MakeDoc(
            "roomNumber", GetString(body, "roomNumber"),
            "roomType", GetString(body, "roomType"),
            "capacity", GetInt(body, "capacity", 1),
            "pricePerNight", GetDouble(body, "pricePerNight", 0.0)
        ),
        "price", bson::MakeDoc(
            "currency", "RUB",
            "pricePerNight", GetDouble(body, "pricePerNight", 0.0),
            "totalPrice", GetDouble(body, "totalPrice", 0.0),
            "discount", 0
        ),
        "payment", bson::MakeDoc(
            "status", "pending",
            "method", body.HasMember("paymentMethod") ? GetString(body, "paymentMethod") : "card"
        ),
        "events", bson::MakeArray(
            bson::MakeDoc(
                "type", "created",
                "message", "Booking was created from API",
                "createdAt", bson::Timestamp{}
            )
        ),
        "createdAt", bson::Timestamp{},
        "updatedAt", bson::Timestamp{}
    );

    bookings.InsertOne(document);

    userver::formats::json::ValueBuilder result;
    result["bookingId"] = booking_id;
    result["status"] = "created";
    return result.ExtractValue();
}

userver::formats::json::Value MongoStorage::GetBooking(const std::string& booking_id) const {
    auto bookings = pool_->GetCollection("bookings");
    const auto doc = bookings.FindOne(bson::MakeDoc("bookingId", booking_id));

    if (!doc) {
        userver::formats::json::ValueBuilder result;
        result["error"] = "booking_not_found";
        return result.ExtractValue();
    }

    return ToJson(*doc);
}

userver::formats::json::Value MongoStorage::ListBookingsByUser(const std::string& user_id) const {
    auto bookings = pool_->GetCollection("bookings");
    auto cursor = bookings.Find(
        bson::MakeDoc("userId", user_id),
        mongo::options::Sort(bson::MakeDoc("createdAt", -1))
    );

    auto arr = userver::formats::json::ValueBuilder(userver::formats::json::Type::kArray);
    for (const auto& doc : cursor) {
        arr.PushBack(ToJson(doc));
    }

    userver::formats::json::ValueBuilder result;
    result["bookings"] = arr.ExtractValue();
    return result.ExtractValue();
}

bool MongoStorage::CancelBooking(const std::string& booking_id, const std::string& user_id) const {
    auto bookings = pool_->GetCollection("bookings");

    const auto result = bookings.UpdateOne(
        bson::MakeDoc(
            "bookingId", booking_id,
            "userId", user_id,
            "status", bson::MakeDoc("$ne", "cancelled")
        ),
        bson::MakeDoc(
            "$set", bson::MakeDoc(
                "status", "cancelled",
                "payment.status", "refunded",
                "updatedAt", bson::Timestamp{}
            ),
            "$push", bson::MakeDoc(
                "events", bson::MakeDoc(
                    "type", "cancelled",
                    "message", "Booking was cancelled from API",
                    "createdAt", bson::Timestamp{}
                )
            )
        )
    );

    return result.ModifiedCount() > 0;
}

userver::formats::json::Value MongoStorage::CreateReview(const userver::formats::json::Value& body) const {
    auto reviews = pool_->GetCollection("reviews");

    const auto review_id = GetString(body, "reviewId");

    reviews.InsertOne(bson::MakeDoc(
        "reviewId", review_id,
        "userId", GetString(body, "userId"),
        "hotelId", GetString(body, "hotelId"),
        "bookingId", GetString(body, "bookingId"),
        "rating", GetInt(body, "rating", 5),
        "comment", GetString(body, "comment"),
        "pros", bson::MakeArray(),
        "cons", bson::MakeArray(),
        "travelerType", body.HasMember("travelerType") ? GetString(body, "travelerType") : "couple",
        "moderation", bson::MakeDoc(
            "status", "pending",
            "checkedBy", bson::Value{},
            "checkedAt", bson::Value{}
        ),
        "createdAt", bson::Timestamp{}
    ));

    userver::formats::json::ValueBuilder result;
    result["reviewId"] = review_id;
    result["status"] = "pending";
    return result.ExtractValue();
}

userver::formats::json::Value MongoStorage::ListReviewsByHotel(const std::string& hotel_id) const {
    auto reviews = pool_->GetCollection("reviews");

    auto cursor = reviews.Find(
        bson::MakeDoc("hotelId", hotel_id),
        mongo::options::Sort(bson::MakeDoc("createdAt", -1))
    );

    auto arr = userver::formats::json::ValueBuilder(userver::formats::json::Type::kArray);
    for (const auto& doc : cursor) {
        arr.PushBack(ToJson(doc));
    }

    userver::formats::json::ValueBuilder result;
    result["reviews"] = arr.ExtractValue();
    return result.ExtractValue();
}

} 
