#pragma once

#include "ApiHandler.hpp" // for Error, TODO: later remove

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <string>
#include <vector>
#include <memory>
#include <expected>

namespace WayHome {

struct RoutePoint {
    std::string code;
    std::string type;
    std::string title;
    std::string station_type;
};

struct Transfer {
    uint32_t duration;
    
    RoutePoint station1;
    RoutePoint station2;

    std::string settlement;
    std::string next_transport_type;
};

struct Thread {
    RoutePoint start_point;
    RoutePoint end_point;

    std::string vehicle;
    std::string number;
    std::string transport_type;
    std::string carrier_name;
    std::string departure_time;
    std::string arrival_time;
};

class Route {
public:
    bool BuildFromJson(const json& segment);

private:
    std::vector<Thread> threads_;
    std::vector<Transfer> transfers_;

    RoutePoint start_point_;
    RoutePoint end_point_;

    std::string departure_time_;
    std::string arrival_time_;

    uint32_t duration_;

    bool has_transfers_ = false;

    std::expected<RoutePoint, Error> ParseRoutePoint(const json& obj);

    bool AddThread(const json& segment, const RoutePoint& start, const RoutePoint& end);
    bool AddTransfer(const json& transfer_obj);

    bool BuildWithoutTransfers(const json& segment);
    bool BuildWithTransfers(const json& segment);
};
    
} // namespace WayHome

