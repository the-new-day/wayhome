#include "Route.hpp"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace WayHome {

bool Route::BuildFromJson(const json& segment) {
    if (!segment.contains("has_transfers")) {
        return false;
    }

    if (segment.at("has_transfers")) {
        return BuildWithTransfers(segment);
    }

    return BuildWithoutTransfers(segment);
}

std::expected<RoutePoint, Error> Route::ParseRoutePoint(const json& obj) {
    if (!obj.contains("code") 
    || !obj.contains("station_type")
    || !obj.contains("title")
    || !obj.contains("type")) {
        return std::unexpected{Error{"Invalid JSON object", ErrorType::kDataError}};
    }

    RoutePoint point;

    point.code = obj["code"];
    point.station_type = obj["station_type"];
    point.title = obj["title"];
    point.type = obj["type"];

    return point;
}

bool Route::AddThread(const json &segment, const RoutePoint& start, const RoutePoint& end) {
    if (!segment.contains("arrival") || !segment.contains("departure")) {
        return false;
    }

    auto segment_thread = segment["thread"];

    if (!segment_thread.contains("transport_type")) {
        return false;
    }

    Thread thread;

    thread.start_point = start;
    thread.end_point = end;

    thread.arrival_time = segment["arrival"];
    thread.departure_time = segment["departure"];

    thread.transport_type = segment["transport_type"];
    
    if (segment_thread.contains("vehicle")) {
        thread.vehicle = segment_thread["vehicle"];
    }
    
    if (segment_thread.contains("carrier") && segment_thread["carrier"].contains("title")) {
        thread.carrier_name = segment_thread["carrier"]["title"];
    }
    
    if (segment_thread.contains("number")) {
        thread.number = segment_thread["number"];
    }

    threads_.push_back(std::move(thread));
    return true;
}

bool Route::BuildWithoutTransfers(const json& segment) {
    if (!segment.contains("thread") || !segment.contains("from") || !segment.contains("to")) {
        return false;
    }

    std::expected<RoutePoint, Error> start_point_parse = ParseRoutePoint(segment["from"]);

    if (!start_point_parse.has_value()) {
        return false;
    }

    std::expected<RoutePoint, Error> end_point_parse = ParseRoutePoint(segment["to"]);

    if (!end_point_parse.has_value()) {
        return false;
    }

    if (!segment.contains("departure") || !segment.contains("arrival") || !segment.contains("duration")) {
        return false;
    }

    if (!AddThread(segment, start_point_, end_point_)) {
        return false;
    }

    start_point_ = start_point_parse.value();
    end_point_ = end_point_parse.value();

    has_transfers_ = false;
    departure_time_ = segment["departure"];
    arrival_time_ = segment["arrival"];
    duration_ = segment["duration"];

    return true;
}

bool Route::BuildWithTransfers(const json& segment) {
    if (!segment.contains("transfers") 
    || !segment.contains("details") 
    || !segment.contains("departure_from") 
    || !segment.contains("arrival_to")) {
        return false;
    }

    std::expected<RoutePoint, Error> start_point_parse = ParseRoutePoint(segment["departure_from"]);

    if (!start_point_parse.has_value()) {
        return false;
    }

    std::expected<RoutePoint, Error> end_point_parse = ParseRoutePoint(segment["arrival_to"]);

    if (!end_point_parse.has_value()) {
        return false;
    }

    auto details_obj = segment["details"];

    for (const auto& detail_obj : details_obj) {
        if (detail_obj.contains("is_transfer") && detail_obj["is_transfer"]) {
            if (!AddTransfer(detail_obj)) {
                return false;
            }

            continue;
        }
        
        if (!detail_obj.contains("thread")) {
            return false;
        }

        std::expected<RoutePoint, Error> start_point_parse = ParseRoutePoint(detail_obj["from"]);

        if (!start_point_parse.has_value()) {
            return false;
        }

        std::expected<RoutePoint, Error> end_point_parse = ParseRoutePoint(detail_obj["to"]);

        if (!end_point_parse.has_value()) {
            return false;
        }

        if (!AddThread(detail_obj, start_point_parse.value(), end_point_parse.value())) {
            return false;
        }
    }

    start_point_ = start_point_parse.value();
    end_point_ = end_point_parse.value();

    departure_time_ = segment["departure"];
    arrival_time_ = segment["arrival"];
    duration_ = segment["duration"];

    return true;
}

bool Route::AddTransfer(const json& transfer_obj) {
    if (!transfer_obj.contains("duration")) {
        return false;
    }

    if (!transfer_obj.contains("transfer_point")) {
        return false;
    }

    auto transfer_point_obj = transfer_obj["transfer_point"];

    if (!transfer_point_obj.contains("settlement")) {
        return false;
    }

    auto transfer_from_obj = transfer_obj["transfer_from"];

    if (!transfer_from_obj.contains("code") 
    || !transfer_from_obj.contains("type") 
    || !transfer_from_obj.contains("title") 
    || !transfer_from_obj.contains("station_type")) {
        return false;
    }

    auto transfer_to_obj = transfer_obj["transfer_to"];

    if (!transfer_to_obj.contains("code") 
    || !transfer_to_obj.contains("type") 
    || !transfer_to_obj.contains("title") 
    || !transfer_to_obj.contains("station_type") 
    || !transfer_to_obj.contains("transport_type")) {
        return false;
    }

    Transfer transfer;

    transfer.duration = transfer_obj["duration"];
    transfer.settlement = transfer_obj["settlement"];
    transfer.next_transport_type = transfer_to_obj["transport_type"];

    transfer.station1 = {
        transfer_from_obj["code"],
        transfer_from_obj["type"],
        transfer_from_obj["title"],
        transfer_from_obj["station_type"]
    };

    transfer.station2 = {
        transfer_to_obj["code"],
        transfer_to_obj["type"],
        transfer_to_obj["title"],
        transfer_to_obj["station_type"]
    };

    transfers_.push_back(std::move(transfer));
    return false;
}

} // namespace WayHome
