#include "handlers.hpp"
#include "models.hpp"
#include "auth.hpp"
#include <nlohmann/json.hpp>
#include <iostream>

static std::string lastLoggedUser; 

void AuthHandler::HandleRequest(const userver::server::http::Request& req,
                                userver::server::http::Response& resp) {
    try {
        auto body = nlohmann::json::parse(req.GetBody());
        int id = UserStore::Instance().AddUser(body);
        resp.SetStatus(201);
        resp.Send("{\"id\":" + std::to_string(id) + "}");
    } catch (const std::exception& e) {
        resp.SetStatus(400);
        resp.Send("{\"error\":\"" + std::string(e.what()) + "\"}");
    }
}

void LoginHandler::HandleRequest(const userver::server::http::Request& req,
                                 userver::server::http::Response& resp) {
    auto body = nlohmann::json::parse(req.GetBody());
    std::string token = Auth::Login(body["login"], body["password"]);
    if (!token.empty()) {
        lastLoggedUser = body["login"];
        resp.SetStatus(200);
        resp.Send("{\"token\":\"" + token + "\"}");
    } else {
        resp.SetStatus(401);
        resp.Send("{\"error\":\"Invalid credentials\"}");
    }
}

void HotelHandler::HandleRequest(const userver::server::http::Request& req,
                                 userver::server::http::Response& resp) {
    if (req.GetMethod() == "POST") {
        auto body = nlohmann::json::parse(req.GetBody());
        int hid = HotelStore::Instance().AddHotel(body);
        resp.SetStatus(201);
        resp.Send("{\"hotel_id\":" + std::to_string(hid) + "}");
    } else if (req.GetMethod() == "GET") {
        auto hotels = HotelStore::Instance().ListHotels();
        resp.SetStatus(200);
        resp.Send(hotels.dump());
    } else {
        resp.SetStatus(405);
        resp.Send("{\"error\":\"Method Not Allowed\"}");
    }
}

void BookingHandler::HandleRequest(const userver::server::http::Request& req,
                                   userver::server::http::Response& resp) {
    std::string authHeader = req.GetHeader("Authorization");
    if (!Auth::CheckToken(authHeader)) {
        resp.SetStatus(403);
        resp.Send("{\"error\":\"Forbidden\"}");
        return;
    }

    if (req.GetMethod() == "POST") {
        auto body = nlohmann::json::parse(req.GetBody());
        int bid = BookingStore::Instance().CreateBooking(body, lastLoggedUser);
        resp.SetStatus(201);
        resp.Send("{\"booking_id\":" + std::to_string(bid) + "}");
    } else if (req.GetMethod() == "DELETE") {
        auto body = nlohmann::json::parse(req.GetBody());
        bool ok = BookingStore::Instance().CancelBooking(body["booking_id"], lastLoggedUser);
        resp.SetStatus(ok ? 200 : 404);
        resp.Send(ok ? "{\"status\":\"cancelled\"}" : "{\"error\":\"Not found\"}");
    } else {
        resp.SetStatus(405);
        resp.Send("{\"error\":\"Method Not Allowed\"}");
    }
}
