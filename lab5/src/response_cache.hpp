#pragma once

#include <chrono>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

#include <userver/components/component_base.hpp>

namespace hotel {

class ResponseCache final : public userver::components::ComponentBase {
public:
    static constexpr std::string_view kName = "response-cache";

    using ComponentBase::ComponentBase;

    std::optional<std::string> Get(const std::string& key) const;

    void Put(const std::string& key,
             std::string value,
             std::chrono::seconds ttl);

    void InvalidatePrefix(std::string_view prefix);

    void Clear();

private:
    struct Entry {
        std::string value;
        std::chrono::steady_clock::time_point expires_at;
    };

    mutable std::mutex mutex_;
    mutable std::unordered_map<std::string, Entry> entries_;
};

} 
