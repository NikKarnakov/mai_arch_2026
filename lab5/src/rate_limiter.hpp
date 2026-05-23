#pragma once

#include <chrono>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>

#include <userver/components/component_base.hpp>

namespace hotel {

class RateLimiter final : public userver::components::ComponentBase {
public:
    static constexpr std::string_view kName = "rate-limiter";

    struct Decision {
        bool allowed{};
        int limit{};
        int remaining{};
        int reset_after_seconds{};
    };

    using ComponentBase::ComponentBase;

    Decision Consume(const std::string& key,
                     int capacity,
                     std::chrono::seconds refill_period);

private:
    struct Bucket {
        double tokens{};
        std::chrono::steady_clock::time_point updated_at{};
    };

    mutable std::mutex mutex_;
    std::unordered_map<std::string, Bucket> buckets_;
};

}
