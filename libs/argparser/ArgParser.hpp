#pragma once

#include "SpecificArgument.hpp"

#include <string>
#include <vector>
#include <map>
#include <cstdint>
#include <optional>

#define ARGPARSER_ADD_ARGUMENT(NewName, Type) \
inline SpecificArgument<Type>& NewName(char short_name, \
                                       const std::string& long_name, \
                                       const std::string& description = "") { \
    return AddArgument<Type>(short_name, long_name, description); \
} \
\
inline SpecificArgument<Type>& NewName(const std::string& long_name, \
                                       const std::string& description = "") { \
    return AddArgument<Type>(kNoShortName, long_name, description); \
} \

#define ARGPARSER_GET_VALUE(NewName, Type) \
inline Type NewName(const std::string& long_name, size_t index = 0) const { \
    return GetValue<Type>(long_name, index).value(); \
} \

namespace ArgumentParser {

class ArgParser {
public:
    explicit ArgParser(const std::string& program_name, const std::string& program_description = "");
    ~ArgParser();

    ArgParser(const ArgParser&) = delete;
    ArgParser& operator=(const ArgParser&) = delete;

    template<typename T>
    SpecificArgument<T>& AddArgument(char short_name,
                                     const std::string& long_name,
                                     const std::string& description = "");

    template<typename T>
    SpecificArgument<T>& AddArgument(const std::string& long_name,
                                     const std::string& description = "");

    std::optional<ArgumentStatus> GetValueStatus(const std::string& long_name) const;

    template<typename T>
    std::optional<T> GetValue(const std::string& long_name, size_t index = 0) const;

    bool Parse(const std::vector<std::string>& argv);
    bool Parse(const std::vector<std::string_view>& argv);
    bool Parse(int argc, char** argv);

    void AddHelp(char short_name,
                 const std::string& long_name,
                 const std::string& description = "");

    void AddHelp(const std::string& long_name,
                 const std::string& description = "");

    bool Help() const;
    std::string HelpDescription() const;

    ParsingError GetError() const;
    bool HasError() const;

    template<typename T>
    void SetTypeAlias(const std::string& alias);

    std::optional<size_t> GetValuesSet(const std::string& long_name) const;

    // The following names are added only to match the interface in the tests.
    // They are unsafe, exceptions may be thrown.
    // It's better to use AddArgument<type> and GetValue<type> and check the return value.

    ARGPARSER_ADD_ARGUMENT(AddIntArgument, int32_t);
    ARGPARSER_ADD_ARGUMENT(AddStringArgument, std::string);
    ARGPARSER_ADD_ARGUMENT(AddFlag, bool);
    ARGPARSER_ADD_ARGUMENT(AddDoubleArgument, double);

    ARGPARSER_GET_VALUE(GetIntValue, int32_t);
    ARGPARSER_GET_VALUE(GetStringValue, std::string);
    ARGPARSER_GET_VALUE(GetFlag, bool);
    ARGPARSER_GET_VALUE(GetDoubleValue, double);

private:
    std::string program_name_;
    std::string program_description_;

    std::vector<Argument*> arguments_;

    std::map<char, std::string_view> short_names_to_long_;
    std::map<std::string, size_t> arguments_indeces_;

    std::map<std::string_view, std::string> help_description_types_;

    ParsingError error_;

    bool need_help_ = false;
    std::string_view help_argument_name_;

    void RefreshParser();

    std::vector<std::string_view> GetLongNames(std::string_view argument) const;

    void ParsePositionalArguments(const std::vector<std::string_view>& argv,
                                  const std::vector<size_t>& positions);

    bool HandleErrors();

    std::string GetArgumentDescription(const Argument* argument,
                                       size_t max_argument_names_length) const;

    std::string GetArgumentNamesDescription(const Argument* argument) const;
};

template<typename T>
SpecificArgument<T>& ArgParser::AddArgument(char short_name,
                                            const std::string& long_name,
                                            const std::string& description) {
    auto* argument = new SpecificArgument<T>(short_name, long_name, description);

    if (arguments_indeces_.contains(long_name)) {
        char arg_short_name = arguments_[arguments_indeces_.at(long_name)]->GetShortName();
        short_names_to_long_.erase(arg_short_name);
        short_names_to_long_[short_name] = long_name;

        delete arguments_[arguments_indeces_.at(long_name)];
        arguments_[arguments_indeces_.at(long_name)] = argument;

        return *argument;
    }

    arguments_indeces_[long_name] = arguments_.size();
    arguments_.push_back(argument);
    short_names_to_long_[short_name] = long_name;

    return *argument;
}

template<typename T>
SpecificArgument<T>& ArgParser::AddArgument(const std::string& long_name,
                                            const std::string& description) {
    return AddArgument<T>(kNoShortName, long_name, description);
}

template <typename T>
std::optional<T> ArgParser::GetValue(const std::string& long_name, size_t index) const {
    if (!arguments_indeces_.contains(long_name)) {
        return std::nullopt;
    }

    size_t argument_index = arguments_indeces_.at(long_name);
    auto* argument = static_cast<SpecificArgument<T>*>(arguments_.at(argument_index));

    return argument->GetValue(index);
}

template <typename T>
void ArgParser::SetTypeAlias(const std::string& alias) {
    help_description_types_[typeid(T).name()] = alias;
}

} // namespace ArgumentParser

#undef ARGPARSER_ADD_ARGUMENT
#undef ARGPARSER_GET_VALUE
