#include "response_cache.hpp"

namespace hotel {

std::optional<std::string> ResponseCache::Get(const std::string& key) const {
    const auto now = std::chrono::steady_clock::now();

    std::lock_guard lock(mutex_);

    const auto it = entries_.find(key);
    if (it == entries_.end()) {
        return std::nullopt;
    }

    if (it->second.expires_at <= now) {
        entries_.erase(it);
        return std::nullopt;
    }

    return it->second.value;
}

void ResponseCache::Put(const std::string& key,
                        std::string value,
                        std::chrono::seconds ttl) {
    const auto expires_at = std::chrono::steady_clock::now() + ttl;

    std::lock_guard lock(mutex_);

    entries_[key] = Entry{
        std::move(value),
        expires_at
    };
}

void ResponseCache::InvalidatePrefix(std::string_view prefix) {
    std::lock_guard lock(mutex_);

    for (auto it = entries_.begin(); it != entries_.end();) {
        if (it->first.rfind(prefix, 0) == 0) {
            it = entries_.erase(it);
        } else {
            ++it;
        }
    }
}

void ResponseCache::Clear() {
    std::lock_guard lock(mutex_);
    entries_.clear();
}

} 
