#pragma once

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <string>
#include <utility>

namespace WayHome {

class CacheHandler {
public:
    CacheHandler(std::string cache_dir, uint32_t ttl_seconds)
        : cache_dir_(std::move(cache_dir))
        , ttl_seconds_(ttl_seconds) {}

    bool IsCacheExpired(const std::string& filename);
    bool UpdateCache(const json& obj, const std::string& filename);
    bool LoadCache(json& to, const std::string& filename);
    bool ClearAllCache();
    bool ClearExpiredCache();

private:
    std::string cache_dir_;
    uint32_t ttl_seconds_;
};
    
} // namespace WayHome

