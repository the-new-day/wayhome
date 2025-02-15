#pragma once

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <string>
#include <set>
#include <utility>
#include <cstdint>
#include <expected>
#include <optional>

namespace WayHome {

const auto kApiUrl = "https://api.rasp.yandex.net/v3.0/search/";

const std::set<std::string> kAllowedTransportTypes = {
    "plane",
    "train",
    "suburban",
    "bus",
    "water",
    "helicopter"
};

enum class ErrorType {
    kNetworkError,
    kApiError,
    kDataError
};

struct Error {
    std::string message;
    ErrorType type;
};

class ApiHandler {
public:
    explicit ApiHandler(std::string&& apikey, std::string&& from, std::string&& to)
        : apikey_(std::move(apikey))
        , from_(std::move(from))
        , to_(std::move(to)) {}

    explicit ApiHandler(const std::string& apikey, const std::string& from, const std::string& to)
        : apikey_(apikey)
        , from_(from)
        , to_(to) {}

    std::expected<json, Error> MakeRequest();

    void SetDate(std::string date);
    void SetTransportType(std::string type);

    void SetRoutesLimit(uint32_t limit);
    void SetMaxTransfers(uint32_t max_transfers);

    bool AreParametersOk() const;

private:
    const std::string apikey_;
    const std::string from_;
    const std::string to_;

    std::string date_;
    std::string transport_type_;
    uint32_t routes_limit_ = 10;
    uint32_t max_transfers_ = 1;

    std::optional<std::string> GetThreadPointCode(const std::string& point) const;
};
    
} // namespace WayHome

