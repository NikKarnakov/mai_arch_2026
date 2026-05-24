#pragma once

#include <optional>
#include <string>

#include <userver/components/component_base.hpp>
#include <userver/formats/json/value.hpp>
#include <userver/storages/mongo/pool.hpp>

namespace hotel {

class MongoStorage final : public userver::components::ComponentBase {
public:
    static constexpr std::string_view kName = "mongo-storage";

    MongoStorage(const userver::components::ComponentConfig& config,
                 const userver::components::ComponentContext& context);

    userver::formats::json::Value CreateBooking(const userver::formats::json::Value& body) const;
    userver::formats::json::Value GetBooking(const std::string& booking_id) const;
    userver::formats::json::Value ListBookingsByUser(const std::string& user_id) const;
    bool CancelBooking(const std::string& booking_id, const std::string& user_id) const;

    userver::formats::json::Value CreateReview(const userver::formats::json::Value& body) const;
    userver::formats::json::Value ListReviewsByHotel(const std::string& hotel_id) const;

private:
    userver::storages::mongo::PoolPtr pool_;
};

} 
