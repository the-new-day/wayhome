#include "CacheHandler.hpp"

#include <filesystem>
#include <chrono>
#include <fstream>

namespace WayHome {

bool CacheHandler::IsCacheExpired(const std::string& filename) const {
    std::error_code ec;
    auto file_time = std::filesystem::last_write_time(cache_dir_ + '/' + filename, ec);

    if (ec) {
        return true;
    }

    auto now = std::chrono::system_clock::now();
    auto system_file_time = now - (std::filesystem::file_time_type::clock::now() - file_time);
    
    return now - system_file_time >= std::chrono::seconds(ttl_seconds_);
}

bool CacheHandler::UpdateCache(const json& obj, const std::string& filename) const {
    std::error_code ec;
    std::filesystem::create_directory(cache_dir_, ec);

    std::ofstream file(cache_dir_ + '/' + filename);

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

bool CacheHandler::LoadCache(json& to, const std::string& filename) const {
    std::ifstream file(cache_dir_ + '/' + filename);

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

bool CacheHandler::ClearAllCache() const {
    std::error_code ec;
    std::filesystem::remove_all(cache_dir_, ec);
    return !ec;
}

bool CacheHandler::ClearExpiredCache() const {
    if (!std::filesystem::exists(cache_dir_)) {
        return true;
    }

    std::filesystem::path dir{cache_dir_};

    for (const auto& file : std::filesystem::directory_iterator{dir}) {
        if (!IsCacheExpired(file.path().filename().string())) {
            continue;
        }

        std::error_code ec;
        std::filesystem::remove(file, ec);

        if (ec) {
            return false;
        }
    }

    return true;
}

} // namespace WayHome
