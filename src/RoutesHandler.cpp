#include "RoutesHandler.hpp"

#include <iostream> // TODO: remove

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

    routes_.reserve(response_obj["segments"].size());

    auto segments_obj = response_obj["segments"];

    for (auto it = segments_obj.begin(); it != segments_obj.end(); ++it) {
        if (!AddRoute(*it)) {
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

void RoutesHandler::Clear() {
    start_point_ = {};
    end_point_ = {};
    departure_date_ = {};
    routes_.clear();
}

} // namespace WayHome
