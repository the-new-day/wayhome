#pragma once

#include <string>
#include <string_view>
#include <cstddef>
#include <vector>
#include <expected>

namespace ArgumentParser {

const char kNoShortName = -1;

enum class ArgumentStatus {
    kSuccess,
    kNoArgument,
    kInvalidArgument,
    kInsufficient
};

enum class ParsingErrorType {
    kInsufficent,
    kInvalidArgument,
    kUnknownArgument,
    kNoArgument,
    kSuccess
};

struct ParsingError {
    std::string_view argument_string;
    ParsingErrorType status = ParsingErrorType::kSuccess;
    std::string_view argument_name;
};

class Argument {
public:
    virtual std::string_view GetType() const = 0;
    virtual ArgumentStatus GetValueStatus() const = 0;
    virtual size_t GetValuesSet() const = 0;
    virtual const std::string& GetDefaultValueString() const = 0;
    virtual void SetDefaultValueString(const std::string& str) = 0;

    virtual const std::string& GetDescription() const = 0;
    virtual const std::string& GetLongName() const = 0;
    virtual char GetShortName() const = 0;

    virtual bool IsPositional() const = 0;
    virtual bool IsMultiValue() const = 0;
    virtual bool HasDefault() const = 0;
    virtual size_t GetMinimumValues() const = 0;

    virtual bool IsFlag() const = 0;

    virtual std::expected<size_t, ParsingError> ParseArgument(const std::vector<std::string_view>& argv,
                                                              size_t position) = 0;

    virtual void Clear() = 0;
};

} // namespace ArgumentParser
