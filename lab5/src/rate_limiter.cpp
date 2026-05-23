#include "rate_limiter.hpp"

#include <algorithm>
#include <cmath>

namespace hotel {

RateLimiter::Decision RateLimiter::Consume(const std::string& key,
                                           int capacity,
                                           std::chrono::seconds refill_period) {
    const auto now = std::chrono::steady_clock::now();

    std::lock_guard lock(mutex_);

    auto& bucket = buckets_[key];

    if (bucket.updated_at.time_since_epoch().count() == 0) {
        bucket.tokens = static_cast<double>(capacity);
        bucket.updated_at = now;
    }

    const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - bucket.updated_at
    );

    if (elapsed.count() > 0) {
        const auto refill_per_ms =
            static_cast<double>(capacity) /
            static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(refill_period).count());

        bucket.tokens = std::min(
            static_cast<double>(capacity),
            bucket.tokens + refill_per_ms * static_cast<double>(elapsed.count())
        );
        bucket.updated_at = now;
    }

    if (bucket.tokens >= 1.0) {
        bucket.tokens -= 1.0;

        return Decision{
            true,
            capacity,
            static_cast<int>(std::floor(bucket.tokens)),
            0
        };
    }

    const auto refill_per_second =
        static_cast<double>(capacity) / static_cast<double>(refill_period.count());

    const auto seconds_to_next_token = static_cast<int>(
        std::ceil((1.0 - bucket.tokens) / refill_per_second)
    );

    return Decision{
        false,
        capacity,
        0,
        std::max(1, seconds_to_next_token)
    };
}

}  
