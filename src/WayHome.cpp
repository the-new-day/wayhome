#include "WayHome.hpp"

#include <argparser/ArgParser.hpp>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <fstream>
#include <format>
#include <filesystem>
#include <iostream>

namespace WayHome {

WayHome::WayHome(const std::string& apikey, const ApiRouteParameters& parameters) : parameters_(parameters) {
    SetCodeForEndpoints();

    if (!HasError()) {
        api_ = std::make_unique<ApiHandler>(apikey_, parameters_);

        if (!cache_.ClearExpiredCache()) {
            error_ = {"Unable to clear expired cache", ErrorType::kEnvironmentError};
        }
    }
}

WayHome::WayHome(const ApiRouteParameters& parameters) : parameters_(parameters) {
    if (!std::filesystem::exists(kSettingsFilename)) {
        CreateSettingsFile();
        if (!HasError()) {
            error_ = {"Apikey wasn't set in " + kSettingsFilename, ErrorType::kParametersError};
        }

        return;
    }

    ReadSettings();
    SetCodeForEndpoints();

    if (!HasError()) {
        api_ = std::make_unique<ApiHandler>(apikey_, parameters_);

        if (!cache_.ClearExpiredCache()) {
            error_ = {"Unable to clear expired cache", ErrorType::kEnvironmentError};
        }
    }
}

void WayHome::ReadSettings() {
    std::ifstream f(kSettingsFilename);

    if (!f.good()) {
        error_ = {"Unable to open " + kSettingsFilename, ErrorType::kEnvironmentError};
        return;
    }

    json settings_obj = json::parse(f, nullptr, false);

    if (!settings_obj.contains("apikey") || settings_obj["apikey"] == "") {
        error_ = {"No apikey was found in " + kSettingsFilename, ErrorType::kEnvironmentError};
    } else if (!settings_obj["apikey"].is_string()) {
        error_ = {"Apikey must be a string", ErrorType::kEnvironmentError};
    }

    apikey_ = std::move(settings_obj["apikey"]);
}

void WayHome::CreateSettingsFile() const {
    std::ofstream f(kSettingsFilename);

    json settings_obj{
        {"apikey", ""}
    };

    if (!f.good()) {
        error_ = {"Unable to open " + kSettingsFilename, ErrorType::kEnvironmentError};
    } else if (!(f << std::setw(4) << settings_obj)) {
        error_ = {"Unable to create or write to " + kSettingsFilename, ErrorType::kEnvironmentError};
    }
}

bool WayHome::SetCodeForEndpoints() {
    if (parameters_.from.size() < 2 || parameters_.to.size() < 2) {
        error_ = {"Parameters 'from' and 'to' are invalid", ErrorType::kParametersError};
        return false;
    }

    if (!parameters_.from.starts_with('c') && !parameters_.from.starts_with('s')) {
        std::expected<std::string, Error> search_result = code_searcher_.FindCode(parameters_.from);

        if (!search_result.has_value()) {
            search_result.error().message 
                = std::format("Could not find a code for {}; {}", parameters_.from, search_result.error().message);

            error_ = search_result.error();
            return false;
        }

        std::cout << "Found code for " + parameters_.from + ": " + search_result.value() << std::endl;
        parameters_.from = search_result.value();
    }

    if (!parameters_.to.starts_with('c') && !parameters_.to.starts_with('s')) {
        std::expected<std::string, Error> search_result = code_searcher_.FindCode(parameters_.to);

        if (!search_result.has_value()) {
            search_result.error().message 
                = std::format("Could not find a code for {}; {}", parameters_.to, search_result.error().message);

            error_ = search_result.error();
            return false;
        }

        std::cout << "Found code for " + parameters_.to + ": " + search_result.value() << std::endl;
        parameters_.to = search_result.value();
    }

    return true;
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

void WayHome::DumpRoutesToJson(const std::string& filename) const {
    std::ofstream file{filename};

    if (!file.good()) {
        error_ = {"Unable to open the file to dump routes: " + filename, ErrorType::kEnvironmentError};
        return;
    }

    DumpRoutesToJson(file);
}

void WayHome::DumpRoutesPretty(std::ostream& stream) const {
    if (HasError()) {
        return;
    }

    routes_.DumpRoutesPretty(stream, parameters_.max_transfers);
    if (routes_.HasError()) {
        error_ = routes_.GetError();
    }
}

void WayHome::DumpRoutesToJson(std::ostream& stream) const {
    if (HasError()) {
        return;
    }

    routes_.DumpRoutesToJson(stream, parameters_.max_transfers);
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
    std::expected<json, Error> request_result = api_->MakeRoutesRequest();

    if (!request_result.has_value()) {
        error_ = api_->GetError();
        return;
    }

    routes_.BuildFromJson(request_result.value());
    if (routes_.HasError()) {
        error_ = routes_.GetError();
        return;
    }
    
    if (!cache_.UpdateCache(request_result.value(), GetCacheFilename())) {
        error_ = {"Unable to update cache", ErrorType::kEnvironmentError};
    }
}

void WayHome::ClearAllCache() const {
    if (!cache_.ClearAllCache()) {
        error_ = {"Unable to clear all cache", ErrorType::kEnvironmentError};
    }
}

bool WayHome::LoadRoutesFromCache(const std::string& filename) {
    json read_to;
    bool is_reading_successful = cache_.LoadCache(read_to, filename);

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
