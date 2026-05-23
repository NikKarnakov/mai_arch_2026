#include "handlers.hpp"
#include "mongo_storage.hpp"

#include <userver/components/minimal_server_component_list.hpp>
#include <userver/server/handlers/ping.hpp>
#include <userver/storages/mongo/component.hpp>
#include <userver/utils/daemon_run.hpp>

int main(int argc, char* argv[]) {
    auto component_list = userver::components::MinimalServerComponentList()
        .Append<userver::server::handlers::Ping>()
        .Append<userver::components::Mongo>("mongo-hotel-booking")
        .Append<hotel::MongoStorage>()
        .Append<hotel::MongoBookingsHandler>()
        .Append<hotel::MongoReviewsHandler>();

    return userver::utils::DaemonMain(argc, argv, component_list);
}
