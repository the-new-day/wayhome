#pragma once

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <string>
#include <set>
#include <cstdint>
#include <expected>
#include <optional>

namespace WayHome {

const std::string kApiUrl{"https://api.rasp.yandex.net/v3.0/search/"};

struct ApiRouteParameters {
    std::string from;
    std::string to;
    uint32_t limit;
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

    std::expected<json, Error> MakeRequest();

    bool ValidateParameters();

    const Error& GetError() const;

private:
    std::string apikey_;
    ApiRouteParameters parameters_;
    Error error_;

    std::optional<std::string> GetThreadPointCode(const std::string& point) const;
};
    
} // namespace WayHome

