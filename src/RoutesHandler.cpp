#include "RoutesHandler.hpp"

namespace WayHome {

bool RoutesHandler::BuildFromJson(const json& response_obj) {
    Clear();
    
    if (!response_obj.contains("search")) {
        return false;
    }

    auto search_obj = response_obj["search"];

    if (!search_obj.contains("from") || !search_obj.contains("to") || !search_obj.contains("date")) {
        return false;
    }

    std::expected<RoutePoint, Error> from_parse_result = Route::ParseRoutePoint(search_obj["from"]);

    if (!from_parse_result.has_value()) {
        return false;
    }

    std::expected<RoutePoint, Error> to_parse_result = Route::ParseRoutePoint(search_obj["to"]);

    if (!to_parse_result.has_value()) {
        return false;
    }

    auto segments_obj = response_obj["segments"];

    routes_.reserve(segments_obj.size());

    for (const auto& segment : segments_obj) {
        if (!AddRoute(segment)) {
            return false;
        }
    }
    
    start_point_ = from_parse_result.value();
    end_point_ = to_parse_result.value();

    departure_date_ = search_obj["date"];

    return true;
}

bool RoutesHandler::AddRoute(const json& segment) {
    Route route;

    if (!route.BuildFromJson(segment)) {
        return false;
    }

    routes_.push_back(std::move(route));
    return true;
}

const std::vector<Route>& RoutesHandler::GetRoutes() const {
    return routes_;
}

const RoutePoint& RoutesHandler::GetStartPoint() const {
    return start_point_;
}

const RoutePoint& RoutesHandler::GetEndPoint() const {
    return end_point_;
}

void RoutesHandler::DumpRoutesToJson(std::ostream& stream, uint32_t max_transfers) const {
    json obj;
    obj["from"] = {
        {"code", start_point_.code},
        {"title", start_point_.title},
    };
    
    obj["to"] = {
        {"code", end_point_.code},
        {"title", end_point_.title},
    };

    obj["departure"] = departure_date_;

    obj["routes"] = json::array();
    
    for (const Route& route : routes_) {
        json route_obj;
        route_obj["duration"] = route.GetDuration();
        route_obj["transfers"] = route.GetTransfersAmount();

        const RoutePoint& from = route.GetStartPoint();
        const RoutePoint& to = route.GetEndPoint();

        route_obj["from"] = {
            {"code", from.code},
            {"title", from.title},
            {"type", from.type},
            {"station_type", from.station_type}
        };

        route_obj["to"] = {
            {"code", to.code},
            {"title", to.title},
            {"type", to.type},
            {"station_type", to.station_type}
        };

        route_obj["threads"] = json::array();

        const std::vector<Transfer>& transfers = route.GetTransfers();
        size_t k = 0;

        for (const Thread& thread : route.GetThreads()) {
            json thread_obj;
            thread_obj["is_transfer"] = false;

            thread_obj["from"] = {
                {"code", thread.start_point.code},
                {"title", thread.start_point.title},
                {"type", thread.start_point.type},
                {"station_type", thread.start_point.station_type}
            };

            thread_obj["to"] = {
                {"code", thread.end_point.code},
                {"title", thread.end_point.title},
                {"type", thread.end_point.type},
                {"station_type", thread.end_point.station_type}
            };

            thread_obj["vehicle"] = thread.vehicle;
            thread_obj["carrier_name"] = thread.carrier_name;
            thread_obj["transport_type"] = thread.transport_type;
            thread_obj["number"] = thread.number;

            thread_obj["departure_time"] = thread.departure_time;
            thread_obj["arrival_time"] = thread.arrival_time;
            
            thread_obj["duration"] = thread.duration;

            route_obj["threads"].push_back(std::move(thread_obj));

            if (k < transfers.size() && transfers.size() <= max_transfers) {
                json thread_obj;
                thread_obj["is_transfer"] = true;

                thread_obj["from"] = {
                    {"code", transfers[k].station1.code},
                    {"title", transfers[k].station1.title},
                    {"type", transfers[k].station1.type},
                    {"station_type", transfers[k].station1.station_type}
                };

                thread_obj["to"] = {
                    {"code", transfers[k].station2.code},
                    {"title", transfers[k].station2.title},
                    {"type", transfers[k].station2.type},
                    {"station_type", transfers[k].station2.station_type}
                };

                thread_obj["transfer_point"] = {
                    {"code", transfers[k].station2.code},
                    {"title", transfers[k].station2.title},
                    {"type", transfers[k].station2.type},
                    {"station_type", transfers[k].station2.station_type}
                };

                thread_obj["duration"] = transfers[k].duration;
                thread_obj["next_transport_type"] = transfers[k].next_transport_type;

                route_obj["threads"].push_back(std::move(thread_obj));
                ++k;
            }
        }
        
        obj["routes"].push_back(std::move(route_obj));
    }

    stream << std::setw(4) << obj;
}

void RoutesHandler::Clear() {
    start_point_ = {};
    end_point_ = {};
    departure_date_ = {};
    routes_.clear();
}

} // namespace WayHome
