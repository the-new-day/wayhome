#pragma once

#include "Route.hpp"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <string>
#include <vector>
#include <ostream>

namespace WayHome {

class RoutesHandler {
public:
    bool BuildFromJson(const json& response_obj);
    const std::vector<Route>& GetRoutes() const;

    const RoutePoint& GetStartPoint() const;
    const RoutePoint& GetEndPoint() const;

    void DumpRoutesToJson(std::ostream& stream, uint32_t max_transfers) const;
    
    void Clear();

private:
    RoutePoint start_point_;
    RoutePoint end_point_;
    std::string departure_date_;

    std::vector<Route> routes_;

    bool AddRoute(const json& segment);
};
    
} // namespace WayHome

