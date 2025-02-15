#pragma once

#include "ApiHandler.hpp" // for Error, ErrorType

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <string>
#include <vector>
#include <expected>
#include <cstdint>

namespace WayHome {

struct RoutePoint {
    std::string code;
    std::string type;
    std::string title;
    std::string station_type;
};

struct Transfer {
    uint32_t duration;

    RoutePoint transfer_point;
    RoutePoint station1;
    RoutePoint station2;

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
    
    uint32_t duration;
};

class Route {
public:
    bool BuildFromJson(const json& segment);

    const std::string& GetDepartureTime() const;
    const std::string& GetArrivalTime() const;

    bool HasTransfers() const;
    size_t GetTransfersAmount() const;

    const std::vector<Thread>& GetThreads() const;
    const std::vector<Transfer>& GetTransfers() const;

    const RoutePoint& GetStartPoint() const;
    const RoutePoint& GetEndPoint() const;

    uint32_t GetDuration() const;

    static std::expected<RoutePoint, Error> ParseRoutePoint(const json& obj);

private:
    std::vector<Thread> threads_;
    std::vector<Transfer> transfers_;

    RoutePoint start_point_;
    RoutePoint end_point_;

    std::string departure_time_;
    std::string arrival_time_;

    uint32_t duration_;

    bool AddThread(const json& segment, const RoutePoint& start, const RoutePoint& end);
    bool AddTransfer(const json& transfer_obj);

    bool BuildWithoutTransfers(const json& segment);
    bool BuildWithTransfers(const json& segment);
};
    
} // namespace WayHome
