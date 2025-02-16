#pragma once

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <string>

namespace WayHome {

const std::string kCacheDir{"wayhome_cache"};
const uint32_t kCacheHoursTTL = 24 * 7;

class CacheHandler {
public:
    static bool IsCacheExpired(const std::string& filename);
    static bool UpdateCache(const json& obj, const std::string& filename);
    static bool LoadCache(json& to, const std::string& filename);
    static bool ClearAllCache();
    static bool ClearExpiredCache();
};
    
} // namespace WayHome

