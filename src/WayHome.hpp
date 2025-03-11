#pragma once

#include "ApiHandler.hpp"
#include "RoutesHandler.hpp"
#include "CacheHandler.hpp"
#include "CodeSearcher.hpp"

#include <string>
#include <memory>
#include <ostream>

namespace WayHome {

const std::string kSettingsFilename{"wayhome_settings.json"};

const std::string kCacheDir{"wayhome_cache"};
const uint32_t kCacheSecondsTTL = 7 * 24 * 60 * 60;

class WayHome {
public:
    WayHome(const std::string& apikey, const ApiRouteParameters& parameters);
    WayHome(const ApiRouteParameters& parameters);

    void CalculateRoutes();

    void DumpRoutesToJson(std::ostream& stream) const;
    void DumpRoutesToJson(const std::string& filename) const;

    void DumpRoutesPretty(std::ostream& stream) const;

    const std::vector<Route>& GetRoutes() const;

    const RoutePoint& GetStartPoint() const;
    const RoutePoint& GetEndPoint() const;

    const Error& GetError() const;
    bool HasError() const;

    void UpdateRoutesWithAPI();

    void ClearAllCache() const;

private:
    RoutesHandler routes_;
    std::unique_ptr<ApiHandler> api_;
    CacheHandler cache_{kCacheDir, kCacheSecondsTTL};

    CodeSearcher code_searcher_;
    ApiRouteParameters parameters_;

    std::string apikey_;

    mutable Error error_;

    std::string GetCacheFilename() const;

    bool LoadRoutesFromCache(const std::string& filename);

    void ReadSettings();
    void CreateSettingsFile() const;

    bool SetCodeForEndpoints();
};
    
} // namespace WayHome
