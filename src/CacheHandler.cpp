#include "CacheHandler.hpp"

#include <filesystem>
#include <chrono>
#include <fstream>

namespace WayHome {

bool CacheHandler::IsCacheExpired(const std::string& filename) {
    std::error_code ec;
    auto file_time = std::filesystem::last_write_time(kCacheDir + '/' + filename, ec);

    if (ec) {
        return true;
    }

    auto now = std::chrono::system_clock::now();
    auto system_file_time = now - (std::filesystem::file_time_type::clock::now() - file_time);
    
    return now - system_file_time >= std::chrono::hours(kCacheHoursTTL);
}

bool CacheHandler::UpdateCache(const json& obj, const std::string& filename) {
    std::error_code ec;
    std::filesystem::create_directory(kCacheDir, ec);

    std::ofstream file(kCacheDir + '/' + filename);

    if (ec || !file.good()) {
        return false;
    }

    try {
        file << obj;
    } catch (const json::exception& e) {
        return false;
    }

    return file.good();
}

bool CacheHandler::LoadCache(json& to, const std::string& filename) {
    std::ifstream file(kCacheDir + '/' + filename);

    if (!file.good()) {
        return false;
    }

    try {
        to = json::parse(file);
    } catch (const json::exception& e) {
        return false;
    }

    return true;
}

} // namespace WayHome
