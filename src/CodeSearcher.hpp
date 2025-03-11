#pragma once

#include "ApiHandler.hpp"
#include "CacheHandler.hpp"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <expected>
#include <optional>

namespace WayHome {

const std::string kCodesFilename = "wayhome_codes.json";

class CodeSearcher {
public:
    std::expected<std::string, Error> FindCode(const std::string& input) const;

private:
    ApiHandler api_handler;
    CacheHandler cache_handler;

    bool DoesCacheExist() const;

    std::optional<std::string> TryFindingInCache(const std::string& input) const;
    std::expected<std::string, Error> CallApi(const std::string& input) const;

    bool SaveToCache(const std::string& name, const std::string& code) const;
};
    
} // namespace WayHome
