#include "WayHome.hpp"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <fstream>
#include <format>
#include <filesystem>

namespace WayHome {

WayHome::WayHome(const ApiRouteParameters& parameters) : parameters_(parameters) {
    if (!std::filesystem::exists(kSettingsFilename)) {
        json empty_obj;
        empty_obj["apikey"] = "";

        std::ofstream f(kSettingsFilename);

        if (!f.good()) {
            error_ = {"Unable to open " + kSettingsFilename, ErrorType::kEnvironmentError};
        } else if (!(f << std::setw(4) << empty_obj)) {
            error_ = {"Unable to create or write to " + kSettingsFilename, ErrorType::kEnvironmentError};
        } else {
            error_ = {"Apikey wasn't set in " + kSettingsFilename, ErrorType::kParametersError};
        }

        return;
    }

    std::ifstream f(kSettingsFilename);

    if (!f.good()) {
        error_ = {"Unable to open " + kSettingsFilename, ErrorType::kEnvironmentError};
        return;
    }

    json settings_obj = json::parse(f, nullptr, false);

    if (!settings_obj.contains("apikey") || settings_obj["apikey"] == "") {
        error_ = {"No apikey was found in " + kSettingsFilename, ErrorType::kEnvironmentError};
        return;
    } else if (!settings_obj["apikey"].is_string()) {
        error_ = {"Apikey must be a string", ErrorType::kEnvironmentError};
        return;
    }

    api_ = std::make_unique<ApiHandler>(settings_obj["apikey"], parameters);

    if (!CacheHandler::ClearExpiredCache()) {
        error_ = {"Unable to clear expired cache", ErrorType::kEnvironmentError};
    }
}

void WayHome::CalculateRoutes() {
    if (HasError()) {
        return;
    }

    std::string cache_filename = GetCacheFilename();

    if (!cache_.IsCacheExpired(cache_filename) && LoadRoutesFromCache(cache_filename)) {
        return;
    }

    UpdateRoutesWithAPI();
}

void WayHome::DumpRoutesToJson(const std::string& filename) {
    if (HasError()) {
        return;
    }

    std::ofstream file(filename);

    if (!file.good()) {
        error_ = {"Unable to open the file to dump routes: " + filename, ErrorType::kEnvironmentError};
        return;
    }

    routes_.DumpRoutesToJson(file, parameters_.max_transfers);
    if (routes_.HasError()) {
        error_ = routes_.GetError();
    }
}

std::string WayHome::GetCacheFilename() const {
    return std::format(
        "{}_{}_{}_{}_{}transfers.json",
        parameters_.from,
        parameters_.to,
        parameters_.date,
        parameters_.transport_type,
        std::to_string(parameters_.max_transfers)
    );
}

void WayHome::UpdateRoutesWithAPI() {
    std::expected<json, Error> request_result = api_->MakeRequest();

    if (!request_result.has_value()) {
        error_ = api_->GetError();
        return;
    }

    routes_.BuildFromJson(request_result.value());
    if (routes_.HasError()) {
        error_ = routes_.GetError();
        return;
    }
    
    if (!CacheHandler::UpdateCache(request_result.value(), GetCacheFilename())) {
        error_ = {"Unable to update cache", ErrorType::kEnvironmentError};
    }
}

void WayHome::ClearAllCache() {
    if (!CacheHandler::ClearAllCache()) {
        error_ = {"Unable to clear all cache", ErrorType::kEnvironmentError};
    }
}

bool WayHome::LoadRoutesFromCache(const std::string& filename) {
    json read_to;
    bool is_reading_successful = CacheHandler::LoadCache(read_to, filename);

    if (is_reading_successful) {
        routes_.BuildFromJson(read_to);

        if (routes_.HasError()) {
            error_ = routes_.GetError();
            return false;
        }
    } else {
        error_ = {"Unable to load cache", ErrorType::kDataError};
    }

    return is_reading_successful;
}

const std::vector<Route>& WayHome::GetRoutes() const {
    return routes_.GetRoutes();
}

const Error& WayHome::GetError() const {
    return error_;
}

bool WayHome::HasError() const {
    return error_.type != ErrorType::kOk;
}

const RoutePoint& WayHome::GetStartPoint() const {
    return routes_.GetStartPoint();
}

const RoutePoint& WayHome::GetEndPoint() const {
    return routes_.GetEndPoint();
}

} // namespace WayHome
