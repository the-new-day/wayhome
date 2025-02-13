#include "SpecificArgument.hpp"
#include "utils/utils.hpp"

#include <cstdint>
#include <string_view>

namespace ArgumentParser {

template<>
std::optional<int32_t> ParseValue<int32_t>(std::string_view value_string) {
    auto parsing_result = ParseNumber<int32_t>(value_string);
    if (!parsing_result.has_value()) {
        return std::nullopt;
    }

    return parsing_result.value();
}

template<>
std::optional<std::string> ParseValue<std::string>(std::string_view value_string) {
    return std::string(value_string);
}

template<>
std::optional<bool> ParseValue<bool>(std::string_view value_string) {
    if (!value_string.empty()) {
        return std::nullopt;
    }

    return true;
}

template<>
std::optional<double> ParseValue<double>(std::string_view value_string) {
    auto parsing_result = ParseNumber<double>(value_string);
    if (!parsing_result.has_value()) {
        return std::nullopt;
    }

    return parsing_result.value();
}

template<>
std::optional<int64_t> ParseValue<int64_t>(std::string_view value_string) {
    auto parsing_result = ParseNumber<int64_t>(value_string);
    if (!parsing_result.has_value()) {
        return std::nullopt;
    }

    return parsing_result.value();
}

template<>
std::optional<int16_t> ParseValue<int16_t>(std::string_view value_string) {
    auto parsing_result = ParseNumber<int16_t>(value_string);
    if (!parsing_result.has_value()) {
        return std::nullopt;
    }

    return parsing_result.value();
}

template<>
std::optional<uint64_t> ParseValue<uint64_t>(std::string_view value_string) {
    auto parsing_result = ParseNumber<uint64_t>(value_string);
    if (!parsing_result.has_value()) {
        return std::nullopt;
    }

    return parsing_result.value();
}

template<>
std::optional<uint32_t> ParseValue<uint32_t>(std::string_view value_string) {
    auto parsing_result = ParseNumber<uint32_t>(value_string);
    if (!parsing_result.has_value()) {
        return std::nullopt;
    }

    return parsing_result.value();
}

template<>
std::optional<uint16_t> ParseValue<uint16_t>(std::string_view value_string) {
    auto parsing_result = ParseNumber<uint16_t>(value_string);
    if (!parsing_result.has_value()) {
        return std::nullopt;
    }

    return parsing_result.value();
}

template<>
std::optional<uint8_t> ParseValue<uint8_t>(std::string_view value_string) {
    auto parsing_result = ParseNumber<uint8_t>(value_string);
    if (!parsing_result.has_value()) {
        return std::nullopt;
    }

    return parsing_result.value();
}

template<>
std::optional<float> ParseValue<float>(std::string_view value_string) {
    auto parsing_result = ParseNumber<float>(value_string);
    if (!parsing_result.has_value()) {
        return std::nullopt;
    }

    return parsing_result.value();
}

template<>
std::optional<long double> ParseValue<long double>(std::string_view value_string) {
    auto parsing_result = ParseNumber<long double>(value_string);
    if (!parsing_result.has_value()) {
        return std::nullopt;
    }

    return parsing_result.value();
}

template<>
std::optional<char> ParseValue<char>(std::string_view value_string) {
    if (value_string.length() != 1) {
        return std::nullopt;
    }

    return value_string[0];
}

} // namespace ArgumentParser
