#pragma once

#include "ApiHandler.hpp"
#include "RoutesHandler.hpp"
#include "CacheHandler.hpp"

#include <string>
#include <memory>

namespace WayHome {

const std::string kSettingsFilename{"wayhome_settings.json"};
const std::string kResultFilename{"wayhome_routes.json"};

class WayHome {
public:
    WayHome(const std::string& apikey, const ApiRouteParameters& parameters) 
        : api_(std::make_unique<ApiHandler>(apikey, parameters))
        , parameters_(parameters) {}

    WayHome(const ApiRouteParameters& parameters);

    void CalculateRoutes();
    void DumpRoutesToJson(const std::string& filename);

    const std::vector<Route>& GetRoutes() const;

    const RoutePoint& GetStartPoint() const;
    const RoutePoint& GetEndPoint() const;

    const Error& GetError() const;
    bool HasError() const;

    void UpdateRoutesWithAPI();

private:
    RoutesHandler routes_;
    std::unique_ptr<ApiHandler> api_;
    CacheHandler cache_;

    ApiRouteParameters parameters_;

    Error error_;

    std::string GetCacheFilename() const;

    bool LoadRoutesFromCache(const std::string& filename);
};
    
} // namespace WayHome
