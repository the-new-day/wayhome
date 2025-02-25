#pragma once

#include "ApiHandler.hpp"
#include "RoutesHandler.hpp"
#include "CacheHandler.hpp"

#include <string>
#include <memory>
#include <ostream>

namespace WayHome {

const std::string kSettingsFilename{"wayhome_settings.json"};

const std::string kCacheDir{"wayhome_cache"};
const uint32_t kCacheSecondsTTL = 7 * 24 * 60 * 60;

class WayHome {
public:
    WayHome(const std::string& apikey, const ApiRouteParameters& parameters) 
        : api_(std::make_unique<ApiHandler>(apikey, parameters))
        , parameters_(parameters) {}

    WayHome(const ApiRouteParameters& parameters);

    void CalculateRoutes();
    void DumpRoutesToJson(std::ostream& stream);
    void DumpRoutesToJson(const std::string& filename);

    void DumpRoutesPretty(std::ostream& stream);

    const std::vector<Route>& GetRoutes() const;

    const RoutePoint& GetStartPoint() const;
    const RoutePoint& GetEndPoint() const;

    const Error& GetError() const;
    bool HasError() const;

    void UpdateRoutesWithAPI();

    void ClearAllCache();

private:
    RoutesHandler routes_;
    std::unique_ptr<ApiHandler> api_;
    CacheHandler cache_{kCacheDir, kCacheSecondsTTL};

    ApiRouteParameters parameters_;

    std::string apikey_;

    Error error_;

    std::string GetCacheFilename() const;

    bool LoadRoutesFromCache(const std::string& filename);

    void ReadSettings();
    void CreateSettingsFile();
};
    
} // namespace WayHome
