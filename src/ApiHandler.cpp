#include "ApiHandler.hpp"

#include <cpr/cpr.h>

#include <string_view>
#include <algorithm>

namespace WayHome {

std::expected<json, Error> ApiHandler::MakeRequest() {
    if (!ValidateParameters()) {
        return std::unexpected{Error{"Invalid parameters", ErrorType::kParametersError}};
    }

    cpr::Response r = cpr::Get(
        cpr::Url{kApiUrl},
        cpr::Parameters{
            {"apikey", apikey_},
            {"from", GetThreadPointCode(parameters_.from).value()},
            {"to", GetThreadPointCode(parameters_.to).value()},
            {"transfers", (parameters_.max_transfers == 0 ? "false" : "true")},
            {"transport_types", parameters_.transport_type},
            {"date", parameters_.date},
            {"format", "json"}
        }
    );

    if (r.status_code >= 300 && r.status_code < 400 || r.status_code >= 500) {
        error_ = {"API error: " + r.error.message, ErrorType::kApiError};
        return std::unexpected{error_};
    } else if (r.status_code >= 400 && r.status_code < 500) {
        error_ = {"Parameters error in request: " + r.url.str(), ErrorType::kParametersError};
        return std::unexpected{error_};
    } else if (r.status_code != 200) {
        error_ = {"Network error", ErrorType::kNetworkError};
        return std::unexpected{error_};
    }

    try {
        return json::parse(r.text);
    } catch (const json::exception& e) {
        return std::unexpected{Error{e.what(), ErrorType::kDataError}};
    }
}

bool ApiHandler::ValidateParameters() {
    if (!kAllowedTransportTypes.contains(parameters_.transport_type)) {
        error_ = {"Invalid transport type", ErrorType::kParametersError};
        return false;
    }

    std::string_view date{parameters_.date};

    // YYYY-MM-DD
    if (date.size() != 10 || date[4] != '-' || date[7] != '-') {
        error_ = {"Invalid date format, must be YYYY-MM-DD (-) " + parameters_.date, ErrorType::kParametersError};
        return false;
    }

    std::string_view year = date.substr(0, 4);
    std::string_view month = date.substr(5, 2);
    std::string_view day = date.substr(8, 2);

    auto check_if_digit = [](char ch) { return std::isdigit(ch); };

    if (!std::all_of(year.begin(), year.end(), check_if_digit)
    || !std::all_of(month.begin(), month.end(), check_if_digit)
    || !std::all_of(day.begin(), day.end(), check_if_digit)) {
        error_ = {"Invalid date format, must be YYYY-MM-DD", ErrorType::kParametersError};
        return false;
    }

    if (month > "12" || month < "01" || day < "01" || day > "31" || year < "2025") {
        error_ = {"Invalid date", ErrorType::kParametersError};
        return false;
    }

    return true;
}

std::optional<std::string> ApiHandler::GetThreadPointCode(const std::string& point) const {
    return point; // TODO:
}

const Error& ApiHandler::GetError() const {
    return error_;
}

} // namespace WayHome

