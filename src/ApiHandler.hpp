#pragma once

#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <string>
#include <set>
#include <cstdint>
#include <expected>
#include <optional>
#include <utility>

namespace WayHome {

const std::string kApiUrl{"https://api.rasp.yandex.net/v3.0/search/"};
const std::string kSuggestsUrl{"https://suggests.rasp.yandex.net/all_suggests"};

struct ApiRouteParameters {
    std::string from;
    std::string to;
    std::string transport_type;
    std::string date;
    uint32_t max_transfers;
};

const std::set<std::string> kAllowedTransportTypes = {
    "plane",
    "train",
    "suburban",
    "bus",
    "water",
    "helicopter",
    "" // all types
};

enum class ErrorType {
    kNetworkError,
    kApiError,
    kDataError,
    kParametersError,
    kEnvironmentError,
    kOk
};

struct Error {
    std::string message;
    ErrorType type = ErrorType::kOk;
};

class ApiHandler {
public:
    ApiHandler(std::string apikey, ApiRouteParameters parameters)
        : apikey_(std::move(apikey))
        , parameters_(std::move(parameters)) {}

    ApiHandler() = default;

    void SetApikey(std::string apikey);
    void SetParameters(ApiRouteParameters parameters);

    std::expected<json, Error> MakeRoutesRequest() const;
    std::expected<json, Error> MakeSuggestsRequest(const std::string& input) const;

    bool ValidateParameters() const;

    const Error& GetError() const;
    bool HasError() const;

private:
    std::string apikey_;
    ApiRouteParameters parameters_;
    mutable Error error_;

    std::expected<json, Error> ProcessRequest(const cpr::Response& r) const;
    void ProcessRequestErrors(const cpr::Response& r) const;
};
    
} // namespace WayHome
