#include "Route.hpp"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace WayHome {

bool Route::BuildFromJson(const json& segment) {
    if (segment.contains("has_transfers") && segment["has_transfers"]) {
        return BuildWithTransfers(segment);
    }
    
    return BuildWithoutTransfers(segment);
}

std::expected<RoutePoint, Error> Route::ParseRoutePoint(const json& obj) {
    if (!obj.contains("code") 
    || !obj.contains("title")
    || !obj.contains("type")) {
        return std::unexpected{Error{"Unable to parse route: invalid JSON object", ErrorType::kDataError}};
    }

    RoutePoint point;

    if (obj.contains("station_type")) {
        point.station_type = obj["station_type"];
    }

    point.code = obj["code"];
    point.title = obj["title"];
    point.type = obj["type"];

    return point;
}

bool Route::AddThread(const json& segment, const RoutePoint& start, const RoutePoint& end) {
    if (!segment.contains("arrival") || !segment.contains("departure")) {
        error_ = {"Invalid JSON: no \"arrival\" or \"departure\" in segment", ErrorType::kDataError};
        return false;
    }

    auto segment_thread = segment["thread"];

    if (!segment_thread.contains("transport_type")) {
        error_ = {"Invalid JSON: no \"transport_type\" in segment", ErrorType::kDataError};
        return false;
    }

    Thread thread;

    thread.start_point = start;
    thread.end_point = end;

    thread.arrival_time = segment["arrival"];
    thread.departure_time = segment["departure"];

    thread.transport_type = segment_thread["transport_type"];
    
    if (segment_thread.contains("vehicle") && !segment_thread["vehicle"].is_null()) {
        thread.vehicle = segment_thread["vehicle"];
    }
    
    if (segment_thread.contains("carrier") 
    && segment_thread["carrier"].contains("title") 
    && !segment_thread["carrier"]["title"].is_null()) {
        thread.carrier_name = segment_thread["carrier"]["title"];
    }
    
    if (segment_thread.contains("number")
    && !segment_thread["number"].is_null()) {
        thread.number = segment_thread["number"];
    }

    if (segment.contains("duration")) {
        thread.duration = segment["duration"];
    }

    threads_.push_back(std::move(thread));
    return true;
}

bool Route::BuildWithoutTransfers(const json& segment) {
    if (!segment.contains("thread") || !segment.contains("from") || !segment.contains("to")) {
        error_ = {"Invalid JSON: no \"thread\" or \"from\" or \"to\" in segment", ErrorType::kDataError};
        return false;
    }

    std::expected<RoutePoint, Error> start_point_parse = ParseRoutePoint(segment["from"]);

    if (!start_point_parse.has_value()) {
        error_ = start_point_parse.error();
        return false;
    }

    std::expected<RoutePoint, Error> end_point_parse = ParseRoutePoint(segment["to"]);

    if (!end_point_parse.has_value()) {
        error_ = end_point_parse.error();
        return false;
    }

    if (!segment.contains("departure") || !segment.contains("arrival") || !segment.contains("duration")) {
        error_ = {"Invalid JSON: no \"departure\" or \"arrival\" or \"duration\" in segment", ErrorType::kDataError};
        return false;
    }

    if (!AddThread(segment, start_point_parse.value(), end_point_parse.value())) {
        return false;
    }

    start_point_ = start_point_parse.value();
    end_point_ = end_point_parse.value();

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
        error_ = start_point_parse.error();
        return false;
    }

    std::expected<RoutePoint, Error> end_point_parse = ParseRoutePoint(segment["arrival_to"]);

    if (!end_point_parse.has_value()) {
        error_ = end_point_parse.error();
        return false;
    }

    auto details_obj = segment["details"];

    uint32_t total_duration = 0;

    for (const auto& detail_obj : details_obj) {
        if (detail_obj.contains("duration") && detail_obj["duration"].is_number()) {
            total_duration += static_cast<uint32_t>(detail_obj["duration"]);
        }

        if (detail_obj.contains("is_transfer") && detail_obj["is_transfer"]) {
            if (!AddTransfer(detail_obj)) {
                return false;
            }
            
            continue;
        }
        
        if (!detail_obj.contains("thread")) {
            error_ = {"Invalid JSON: no \"thread\" in segment", ErrorType::kDataError};
            return false;
        }

        std::expected<RoutePoint, Error> detail_start_point_parse = ParseRoutePoint(detail_obj["from"]);

        if (!detail_start_point_parse.has_value()) {
            error_ = detail_start_point_parse.error();
            return false;
        }

        std::expected<RoutePoint, Error> detail_end_point_parse = ParseRoutePoint(detail_obj["to"]);

        if (!detail_end_point_parse.has_value()) {  
            error_ = detail_end_point_parse.error();
            return false;
        }

        if (!AddThread(detail_obj, detail_start_point_parse.value(), detail_end_point_parse.value())) {
            return false;
        }
    }

    start_point_ = start_point_parse.value();
    end_point_ = end_point_parse.value();

    departure_time_ = segment["departure"];
    arrival_time_ = segment["arrival"];
    duration_ = total_duration;

    return true;
}

bool Route::AddTransfer(const json& transfer_obj) {
    if (!transfer_obj.contains("duration")) {
        error_ = {"Invalid JSON: no \"duration\" in segment", ErrorType::kDataError};
        return false;
    }

    if (!transfer_obj.contains("transfer_point")) {
        error_ = {"Invalid JSON: no \"transfer_point\" in segment", ErrorType::kDataError};
        return false;
    }

    auto transfer_point_parse = ParseRoutePoint(transfer_obj["transfer_point"]);

    if (!transfer_point_parse.has_value()) {
        error_ = transfer_point_parse.error();
        return false;
    }

    Transfer transfer;

    if (transfer_obj.contains("transfer_from") && transfer_obj.contains("transfer_to")
    && !transfer_obj["transfer_from"].is_null() && !transfer_obj["transfer_to"].is_null()) {
        auto transfer_from_obj = transfer_obj["transfer_from"];

        auto transfer_from_parse = ParseRoutePoint(transfer_obj["transfer_from"]);

        if (!transfer_from_parse.has_value()) {
            error_ = transfer_from_parse.error();
            return false;
        }

        auto transfer_to_parse = ParseRoutePoint(transfer_obj["transfer_to"]);

        if (!transfer_to_parse.has_value()) {
            error_ = transfer_to_parse.error();
            return false;
        }

        transfer.next_transport_type = transfer_obj["transfer_to"]["transport_type"];

        transfer.station1 = transfer_from_parse.value();
        transfer.station2 = transfer_to_parse.value();
    } else {
        transfer.next_transport_type = transfer_obj["transfer_point"].contains("transport_type") 
            ? transfer_obj["transfer_point"]["transport_type"] : "";

        transfer.station1 = transfer_point_parse.value();
        transfer.station2 = transfer_point_parse.value();
    }

    transfer.duration = transfer_obj["duration"];
    transfer.transfer_point = transfer_point_parse.value();

    transfers_.push_back(std::move(transfer));
    return true;
}

const std::string& Route::GetArrivalTime() const {
    return arrival_time_;
}

const std::string& Route::GetDepartureTime() const {
    return departure_time_;
}

bool Route::HasTransfers() const {
    return !transfers_.empty();
}

size_t Route::GetTransfersAmount() const {
    return transfers_.size();
}

const std::vector<Thread>& Route::GetThreads() const {
    return threads_;
}

const std::vector<Transfer>& Route::GetTransfers() const {
    return transfers_;
}

const RoutePoint& Route::GetStartPoint() const {
    return start_point_;
}

const RoutePoint& Route::GetEndPoint() const {
    return end_point_;
}

uint32_t Route::GetDuration() const {
    return duration_;
}

const Error& Route::GetError() const {
    return error_;
}

bool Route::HasError() const {
    return error_.type != ErrorType::kOk;
}

} // namespace WayHome
