#include "WayHome.hpp"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <fstream>

#include <iostream>

namespace WayHome {

WayHome::WayHome(const ApiRouteParameters& parameters) : parameters_(parameters) {
    std::ifstream f(kSettingsFilename);

    if (!f.good()) {
        error_ = {"Unable to open " + kSettingsFilename, ErrorType::kEnvironmentError};
        return;
    }

    json settings_obj = json::parse(f, nullptr, false);

    if (!settings_obj.contains("apikey")) {
        error_ = {"No apikey was found in " + kSettingsFilename, ErrorType::kEnvironmentError};
        return;
    } else if (!settings_obj["apikey"].is_string()) {
        error_ = {"Apikey must be a string", ErrorType::kEnvironmentError};
        return;
    }

    api_ = std::make_unique<ApiHandler>(settings_obj["apikey"], parameters);
}

void WayHome::CalculateRoutes() {
    if (HasError()) {
        return;
    }

    // TODO: cache lookup

    std::expected<json, Error> request_result = api_->MakeRequest();

    if (!request_result.has_value()) {
        error_ = api_->GetError();
        return;
    }

    routes_.BuildFromJson(request_result.value());
    if (routes_.HasError()) {
        error_ = routes_.GetError();
    }
}

void WayHome::DumpRoutesToJson(const std::string& filename) {
    if (HasError()) {
        return;
    }

    std::ofstream file(filename);

    if (!file.good()) {
        error_ = {"Unable to open the file: " + filename, ErrorType::kEnvironmentError};
        return;
    }

    routes_.DumpRoutesToJson(file, parameters_.max_transfers);
    if (routes_.HasError()) {
        error_ = routes_.GetError();
    }
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
