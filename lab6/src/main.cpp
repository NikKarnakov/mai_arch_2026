#include "handlers.hpp"
#include "mongo_handlers.hpp"
#include "mongo_storage.hpp"
#include "rate_limiter.hpp"
#include "response_cache.hpp"
#include "storage.hpp"

#include <userver/components/minimal_server_component_list.hpp>
#include <userver/server/handlers/ping.hpp>
#include <userver/storages/mongo/component.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/utils/daemon_run.hpp>

int main(int argc, char* argv[]) {
    auto component_list = userver::components::MinimalServerComponentList()
        .Append<userver::server::handlers::Ping>()
        .Append<userver::components::Postgres>("hotel-booking-db")
        .Append<userver::components::Mongo>("mongo-hotel-booking")
        .Append<hotel::Storage>()
        .Append<hotel::ResponseCache>()
        .Append<hotel::RateLimiter>()
        .Append<hotel::MongoStorage>()
        .Append<hotel::RegisterHandler>()
        .Append<hotel::LoginHandler>()
        .Append<hotel::UsersHandler>()
        .Append<hotel::HotelsHandler>()
        .Append<hotel::RoomsHandler>()
        .Append<hotel::BookingsHandler>()
        .Append<hotel::ReviewsHandler>()
        .Append<hotel::MongoBookingsHandler>()
        .Append<hotel::MongoReviewsHandler>();

    return userver::utils::DaemonMain(argc, argv, component_list);
}
