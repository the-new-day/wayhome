#pragma once

#include <nlohmann/json.hpp>
#include <cpr/cpr.h>

namespace WayHome {

constexpr auto kApiUrl = "https://api.rasp.yandex.net/v3.0/search/";

struct ThreadPoint {
    std::string code;
    std::string title;
    std::string popular_title;
    std::string short_title;
    std::string station_type;
    std::string station_type_name;
    std::string pount_type;
};

struct ApiCallParameters {
    std::string apikey;
    std::string from;
    std::string to;
    std::string date;
    std::string transport_type;
    int32_t limit;
    int32_t transfers;
};

enum class NetworkError {
    kNetworkUnavailable
};

enum class ApiError {
    kWrongKey,
    kWrongParameter
};

class WayHome {
public:
    explicit WayHome(std::string&& apikey) : apikey_(std::move(apikey)) {}
    explicit WayHome(const std::string& apikey) : apikey_(apikey) {}

    std::vector<std::vector<ThreadPoint>> GetRoutes() const;

private:
    const std::string apikey_;
    std::string from_;
    std::string to_;
    std::string date_;
    std::string transport_type_;
    int32_t limit_;
    int32_t transfers_;

    
};
    
} // namespace WayHome

