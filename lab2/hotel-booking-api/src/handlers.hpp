#pragma once
#include <userver/server/handlers/http_handler_base.hpp>

class AuthHandler : public userver::server::handlers::HttpHandlerBase {
public:
    void HandleRequest(const userver::server::http::Request& req,
                       userver::server::http::Response& resp) override;
};

class LoginHandler : public userver::server::handlers::HttpHandlerBase {
public:
    void HandleRequest(const userver::server::http::Request& req,
                       userver::server::http::Response& resp) override;
};

class HotelHandler : public userver::server::handlers::HttpHandlerBase {
public:
    void HandleRequest(const userver::server::http::Request& req,
                       userver::server::http::Response& resp) override;
};

class BookingHandler : public userver::server::handlers::HttpHandlerBase {
public:
    void HandleRequest(const userver::server::http::Request& req,
                       userver::server::http::Response& resp) override;
};
