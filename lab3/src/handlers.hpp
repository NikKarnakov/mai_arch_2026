#pragma once

#include "storage.hpp"

#include <string_view>

#include <userver/components/component_base.hpp>
#include <userver/components/component_context.hpp>
#include <userver/server/handlers/http_handler_json_base.hpp>

namespace hotel {

class BaseJsonHandler : public userver::server::handlers::HttpHandlerJsonBase {
public:
    BaseJsonHandler(const userver::components::ComponentConfig& config,
                    const userver::components::ComponentContext& context);

protected:
    const Storage& StorageRef() const;

private:
    const Storage& storage_;
};

class RegisterHandler final : public BaseJsonHandler {
public:
    static constexpr std::string_view kName = "handler-register";

    using BaseJsonHandler::BaseJsonHandler;

    userver::formats::json::Value HandleRequestJsonThrow(
        const userver::server::http::HttpRequest& request,
        const userver::formats::json::Value& body,
        userver::server::request::RequestContext& context) const override;
};

class LoginHandler final : public BaseJsonHandler {
public:
    static constexpr std::string_view kName = "handler-login";

    using BaseJsonHandler::BaseJsonHandler;

    userver::formats::json::Value HandleRequestJsonThrow(
        const userver::server::http::HttpRequest& request,
        const userver::formats::json::Value& body,
        userver::server::request::RequestContext& context) const override;
};

class HotelsHandler final : public BaseJsonHandler {
public:
    static constexpr std::string_view kName = "handler-hotels";

    using BaseJsonHandler::BaseJsonHandler;

    userver::formats::json::Value HandleRequestJsonThrow(
        const userver::server::http::HttpRequest& request,
        const userver::formats::json::Value& body,
        userver::server::request::RequestContext& context) const override;
};

class RoomsHandler final : public BaseJsonHandler {
public:
    static constexpr std::string_view kName = "handler-rooms";

    using BaseJsonHandler::BaseJsonHandler;

    userver::formats::json::Value HandleRequestJsonThrow(
        const userver::server::http::HttpRequest& request,
        const userver::formats::json::Value& body,
        userver::server::request::RequestContext& context) const override;
};

class BookingsHandler final : public BaseJsonHandler {
public:
    static constexpr std::string_view kName = "handler-bookings";

    using BaseJsonHandler::BaseJsonHandler;

    userver::formats::json::Value HandleRequestJsonThrow(
        const userver::server::http::HttpRequest& request,
        const userver::formats::json::Value& body,
        userver::server::request::RequestContext& context) const override;
};

class ReviewsHandler final : public BaseJsonHandler {
public:
    static constexpr std::string_view kName = "handler-reviews";

    using BaseJsonHandler::BaseJsonHandler;

    userver::formats::json::Value HandleRequestJsonThrow(
        const userver::server::http::HttpRequest& request,
        const userver::formats::json::Value& body,
        userver::server::request::RequestContext& context) const override;
};

}  // namespace hotel
