#pragma once

#include "mongo_storage.hpp"

#include <userver/server/handlers/http_handler_json_base.hpp>

namespace hotel {

class BaseMongoHandler : public userver::server::handlers::HttpHandlerJsonBase {
public:
    BaseMongoHandler(const userver::components::ComponentConfig& config,
                     const userver::components::ComponentContext& context);

protected:
    const MongoStorage& Storage() const;

private:
    const MongoStorage& storage_;
};

class MongoBookingsHandler final : public BaseMongoHandler {
public:
    static constexpr std::string_view kName = "handler-mongo-bookings";

    using BaseMongoHandler::BaseMongoHandler;

    userver::formats::json::Value HandleRequestJsonThrow(
        const userver::server::http::HttpRequest& request,
        const userver::formats::json::Value& body,
        userver::server::request::RequestContext& context) const override;
};

class MongoReviewsHandler final : public BaseMongoHandler {
public:
    static constexpr std::string_view kName = "handler-mongo-reviews";

    using BaseMongoHandler::BaseMongoHandler;

    userver::formats::json::Value HandleRequestJsonThrow(
        const userver::server::http::HttpRequest& request,
        const userver::formats::json::Value& body,
        userver::server::request::RequestContext& context) const override;
};

}  
