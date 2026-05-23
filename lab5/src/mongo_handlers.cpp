#include "mongo_handlers.hpp"

#include <userver/server/http/http_method.hpp>
#include <userver/server/http/http_status.hpp>

namespace hotel {

BaseMongoHandler::BaseMongoHandler(const userver::components::ComponentConfig& config,
                                   const userver::components::ComponentContext& context)
    : HttpHandlerJsonBase(config, context),
      storage_(context.FindComponent<MongoStorage>()) {}

const MongoStorage& BaseMongoHandler::Storage() const {
    return storage_;
}

userver::formats::json::Value MongoBookingsHandler::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value& body,
    userver::server::request::RequestContext&) const {
    const auto method = request.GetMethod();

    if (method == userver::server::http::HttpMethod::kPost) {
        request.GetHttpResponse().SetStatus(userver::server::http::HttpStatus::kCreated);
        return Storage().CreateBooking(body);
    }

    if (method == userver::server::http::HttpMethod::kDelete) {
        const auto ok = Storage().CancelBooking(
            body["bookingId"].As<std::string>(),
            body["userId"].As<std::string>()
        );

        request.GetHttpResponse().SetStatus(
            ok ? userver::server::http::HttpStatus::kOk
               : userver::server::http::HttpStatus::kNotFound
        );

        userver::formats::json::ValueBuilder result;
        result["status"] = ok ? "cancelled" : "not_found";
        return result.ExtractValue();
    }

    const auto booking_id = request.GetArg("bookingId");
    if (!booking_id.empty()) {
        return Storage().GetBooking(booking_id);
    }

    const auto user_id = request.GetArg("userId");
    if (!user_id.empty()) {
        return Storage().ListBookingsByUser(user_id);
    }

    request.GetHttpResponse().SetStatus(userver::server::http::HttpStatus::kBadRequest);

    userver::formats::json::ValueBuilder result;
    result["error"] = "bookingId or userId query parameter is required";
    return result.ExtractValue();
}

userver::formats::json::Value MongoReviewsHandler::HandleRequestJsonThrow(
    const userver::server::http::HttpRequest& request,
    const userver::formats::json::Value& body,
    userver::server::request::RequestContext&) const {
    const auto method = request.GetMethod();

    if (method == userver::server::http::HttpMethod::kPost) {
        request.GetHttpResponse().SetStatus(userver::server::http::HttpStatus::kCreated);
        return Storage().CreateReview(body);
    }

    const auto hotel_id = request.GetArg("hotelId");
    if (hotel_id.empty()) {
        request.GetHttpResponse().SetStatus(userver::server::http::HttpStatus::kBadRequest);

        userver::formats::json::ValueBuilder result;
        result["error"] = "hotelId query parameter is required";
        return result.ExtractValue();
    }

    return Storage().ListReviewsByHotel(hotel_id);
}

}  
