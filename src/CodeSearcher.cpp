#include "CodeSearcher.hpp"

#include <filesystem>
#include <fstream>

namespace WayHome {

std::expected<std::string, Error> CodeSearcher::FindCode(const std::string& input) const {
    std::optional<std::string> in_cache = TryFindingInCache(input);

    if (in_cache.has_value()) {
        return in_cache.value();
    }

    std::expected<std::string, Error> call_result = CallApi(input);

    if (!call_result.has_value()) {
        return std::unexpected{call_result.error()};
    }

    SaveToCache(input, call_result.value());
    return call_result.value();
}

bool CodeSearcher::DoesCacheExist() const {
    return std::filesystem::exists(kCodesFilename);
}

std::optional<std::string> CodeSearcher::TryFindingInCache(const std::string& input) const {
    json cache_obj;

    if (!cache_handler.LoadCache(cache_obj, kCodesFilename) || !cache_obj.contains(input)) {
        return std::nullopt;
    }

    return cache_obj[input];
}

std::expected<std::string, Error> CodeSearcher::CallApi(const std::string& input) const {
    std::expected<json, Error> request_result = api_handler.MakeSuggestsRequest(input);
    
    if (!request_result.has_value()) {
        return std::unexpected{request_result.error()};
    }

    const json& response_obj = request_result.value();

    if (!response_obj.contains("suggests")) {
        return std::unexpected{Error{"API suggestions response doesn't have \"suggests\" object", ErrorType::kApiError}};
    }

    for (const json& suggestion : response_obj["suggests"]) {
        if (!suggestion.contains("title")) {
            return std::unexpected{Error{"API suggestion doesn't have \"title\" field", ErrorType::kApiError}};
        } else if (!suggestion.contains("point_key")) {
            return std::unexpected{Error{"API suggestion doesn't have \"point_key\" field", ErrorType::kApiError}};
        }

        if (suggestion["title"] == input) {
            return suggestion["code"];
        }
    }

    return std::unexpected{Error{"Couldn't find the name", ErrorType::kParametersError}};
}

bool CodeSearcher::SaveToCache(const std::string& name, const std::string& code) const {
    json cache_obj;

    if (cache_handler.LoadCache(cache_obj, kCodesFilename)) {
        cache_obj[name] = code;
        return cache_handler.UpdateCache(cache_obj, kCodesFilename);
    }

    return false;
}

} // namespace WayHome
