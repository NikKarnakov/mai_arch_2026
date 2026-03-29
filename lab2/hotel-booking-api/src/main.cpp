#include <iostream>
#include "handlers.hpp"
#include <userver/server/server.hpp>
#include <userver/server/handlers/http_handler_base.hpp>

int main() {
    std::cout << "Стартуем... Если получится, конечно...\n";

    userver::server::ServerConfig config;
    config.port = 8080;
    userver::server::Server server(config);

    server.RegisterHandler("/auth/register", std::make_shared<AuthHandler>());
    server.RegisterHandler("/auth/login", std::make_shared<LoginHandler>());
    server.RegisterHandler("/hotels", std::make_shared<HotelHandler>());
    server.RegisterHandler("/bookings", std::make_shared<BookingHandler>());

    std::cout << "Есть! Фух...\n";

    server.Run();
    return 0;
}
