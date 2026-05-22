#include "handlers.hpp"
#include "storage.hpp"

#include <userver/components/minimal_server_component_list.hpp>
#include <userver/server/handlers/ping.hpp>
#include <userver/storages/postgres/component.hpp>
#include <userver/utils/daemon_run.hpp>

int main(int argc, char* argv[]) {
    auto component_list = userver::components::MinimalServerComponentList()
        .Append<userver::server::handlers::Ping>()
        .Append<userver::components::Postgres>("hotel-booking-db")
        .Append<hotel::Storage>()
        .Append<hotel::RegisterHandler>()
        .Append<hotel::LoginHandler>()
        .Append<hotel::HotelsHandler>()
        .Append<hotel::RoomsHandler>()
        .Append<hotel::BookingsHandler>()
        .Append<hotel::ReviewsHandler>();

    return userver::utils::DaemonMain(argc, argv, component_list);
}
