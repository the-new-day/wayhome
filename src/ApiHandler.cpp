#include "ApiHandler.hpp"

#include <cpr/cpr.h>

namespace WayHome {

std::expected<json, Error> ApiHandler::MakeRequest() {
    if (!AreParametersOk()) {
        return std::unexpected{Error{"Invalid parameters", ErrorType::kDataError}};
    }

    cpr::Response r = cpr::Get(
        cpr::Url{kApiUrl},
        cpr::Parameters{
            {"apikey", apikey_},
            {"from", GetThreadPointCode(from_).value()},
            {"to", GetThreadPointCode(to_).value()},
            {"limit", std::to_string(routes_limit_)},
            {"transfers", (max_transfers_ == 0 ? "false" : "true")}
        }
    );

    if (r.status_code >= 400 && r.status_code < 500) {
        return std::unexpected{Error{r.error.message, ErrorType::kApiError}};
    } else if (r.status_code != 200) {
        return std::unexpected{Error{r.error.message, ErrorType::kNetworkError}};
    }

    json response = json::parse(r.text);
    
    if (response.contains("error")) {
        return std::unexpected{Error{
            "API Error: " + response["error"]["text"].template get<std::string>(), 
            ErrorType::kApiError
        }};
    }

    return response;
}

bool ApiHandler::AreParametersOk() const {
    return true;
}

std::optional<std::string> ApiHandler::GetThreadPointCode(const std::string& point) const {
    return point;
}

void ApiHandler::SetDate(std::string date) {
    date_ = std::move(date);
}

void ApiHandler::SetTransportType(std::string type) {
    transport_type_ = std::move(type);
}

void ApiHandler::SetRoutesLimit(uint32_t limit) {
    routes_limit_ = limit;
}

void ApiHandler::SetMaxTransfers(uint32_t max_transfers) {
    max_transfers_ = max_transfers;
}

} // namespace WayHome

