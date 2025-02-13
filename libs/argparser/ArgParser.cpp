#include "ArgParser.hpp"

#include <algorithm>
#include <numeric>

namespace ArgumentParser {
    
ArgParser::ArgParser(const std::string& program_name, const std::string& program_description) 
    : program_name_(program_name),
      program_description_(program_description) {
    help_description_types_ = {
        {typeid(int32_t).name(), "int"},
        {typeid(int64_t).name(), "long long"},
        {typeid(int16_t).name(), "short"},
        {typeid(uint64_t).name(), "unsigned long long"},
        {typeid(uint32_t).name(), "unsigned int"},
        {typeid(uint16_t).name(), "unsigned short"},
        {typeid(uint8_t).name(), "unsigned char"},

        {typeid(double).name(), "double"},
        {typeid(float).name(), "float"},
        {typeid(long double).name(), "long double"},

        {typeid(std::string).name(), "string"},
        {typeid(char).name(), "char"},
        
        {typeid(bool).name(), ""},
    };
}

ArgParser::~ArgParser() {
    for (auto* argument : arguments_) {
        delete argument;
    }
}

void ArgParser::RefreshParser() {
    for (auto* argument : arguments_) {
        argument->Clear();
    }

    error_ = ParsingError{};
}

std::vector<std::string_view> ArgParser::GetLongNames(std::string_view argument) const {
    bool is_long = false;

    if (argument.starts_with("--")) {
        argument = argument.substr(2);
        is_long = true;
    } else {
        argument = argument.substr(1);
    }

    size_t equal_sign_index = argument.find('=');
    argument = argument.substr(0, equal_sign_index);

    std::vector<std::string_view> names;

    if (is_long) {
        names.push_back(argument);
    } else if (equal_sign_index != std::string_view::npos && argument.length() > 1) {
        return {};
    } else {
        for (const char short_name : argument) {
            if (!short_names_to_long_.contains(short_name)) {
                continue;
            }

            names.push_back(short_names_to_long_.at(short_name));
        }
    }

    if (!is_long && names.size() < argument.length()) {
        if (short_names_to_long_.contains(argument[0])) {
            return {names[0]};
        }

        return {};
    }

    return names;
}

bool ArgParser::Parse(const std::vector<std::string_view>& argv) {
    RefreshParser();

    std::vector<size_t> unused_positions;

    for (size_t position = 1; position < argv.size(); ++position) {
        std::string_view argument = argv[position];

        if (argument.empty()) {
            continue;
        }

        if (argument == "--") {
            unused_positions.reserve(argv.size() - position - 1);

            for (size_t i = position + 1; i < argv.size(); ++i) {
                unused_positions.push_back(i);
            }

            break;
        }

        if (argument[0] != '-' || argument.length() == 1) {
            unused_positions.push_back(position);
            continue;
        }

        std::vector<std::string_view> long_names = GetLongNames(argument);

        if (long_names.empty()) {
            error_ = {argv[position], ParsingErrorType::kUnknownArgument};
            return false;
        }

        if (argument[1] != '-' && argument.length() > 2 && long_names.size() == 1) {
            Argument* argument = arguments_[arguments_indeces_[std::string(long_names[0])]];
            if (argument->IsFlag()) {
                error_ = ParsingError{argv[position], ParsingErrorType::kUnknownArgument, long_names[0]};
                return false;
            }
        }

        for (std::string_view long_name : long_names) {
            std::string name{long_name};

            if (!arguments_indeces_.contains(name)) {
                error_ = ParsingError{argv[position], ParsingErrorType::kUnknownArgument, long_name};
                return false;
            }

            size_t argument_index = arguments_indeces_.at(name);

            if (arguments_[argument_index]->IsPositional()) {
                error_ = ParsingError{argv[position], ParsingErrorType::kUnknownArgument, long_name};
                return false;
            }

            std::expected<size_t, ParsingError> current_used_positions 
                = arguments_[argument_index]->ParseArgument(argv, position);

            if (!current_used_positions.has_value()) {
                error_ = current_used_positions.error();
                return false;
            }

            if (long_name == help_argument_name_) {
                need_help_ = true;
            }

            position += current_used_positions.value() - 1;
        }
    }

    ParsePositionalArguments(argv, unused_positions);

    if (need_help_) {
        HandleErrors();
        return true;
    }

    return HandleErrors();
}

void ArgParser::ParsePositionalArguments(const std::vector<std::string_view>& argv,
                                         const std::vector<size_t>& positions) {
    std::vector<size_t> positional_args_indeces;
    std::vector<std::string_view> positional_args;

    for (size_t i = 0; i < arguments_.size(); ++i) {
        if (arguments_[i]->IsPositional()) {
            positional_args_indeces.push_back(i);
        }
    }

    for (size_t i = 0; i < positions.size(); ++i) {
        positional_args.push_back(argv[positions[i]]);
    }

    if (positional_args_indeces.empty()) {
        if (!positional_args.empty()) {
            error_ = ParsingError{argv[positions[0]], ParsingErrorType::kUnknownArgument};
        }

        return;
    }

    for (size_t argument_index = 0, position_index = 0;
        position_index < positions.size() && argument_index < positional_args_indeces.size();
        ++argument_index, ++position_index) {
        Argument* argument = arguments_[positional_args_indeces[argument_index]];
        if (argument->IsMultiValue()) {
            while (position_index < positions.size()) {
                std::expected<size_t, ParsingError> current_used_positions 
                    = argument->ParseArgument(argv, positions[position_index]);

                if (!current_used_positions.has_value()) {
                    error_ = current_used_positions.error();
                    return;
                }

                ++position_index;
            }

            return;
        }

        std::expected<size_t, ParsingError> current_used_positions 
            = argument->ParseArgument(argv, positions[position_index]);

        if (!current_used_positions.has_value()) {
            error_ = current_used_positions.error();
            return;
        }
    }
}

bool ArgParser::Parse(int argc, char **argv) {
    std::vector<std::string_view> new_argv;
    new_argv.reserve(argc);

    for (size_t i = 0; i < argc; ++i) {
        new_argv.push_back(argv[i]);
    }

    return Parse(new_argv);
}

bool ArgParser::Parse(const std::vector<std::string>& argv) {
    std::vector<std::string_view> new_argv;
    new_argv.reserve(argv.size());

    for (size_t i = 0; i < argv.size(); ++i) {
        new_argv.push_back(argv[i]);
    }

    return Parse(new_argv);
}

bool ArgParser::HandleErrors() {
    if (error_.status != ParsingErrorType::kSuccess) {
        return false;
    }

    for (const Argument* argument : arguments_) {
        ArgumentStatus status = argument->GetValueStatus();
        if (status == ArgumentStatus::kSuccess) {
            continue;
        }
        
        if (status == ArgumentStatus::kNoArgument) {
            error_.status = ParsingErrorType::kNoArgument;
        } else if (status == ArgumentStatus::kInsufficient) {
            error_.status = ParsingErrorType::kInsufficent;
        }
        
        error_.argument_name = argument->GetLongName();
        return false;
    }

    return true;
}

void ArgParser::AddHelp(char short_name, const std::string& long_name, const std::string& description) {
    AddFlag(short_name, long_name, description);
    help_argument_name_ = long_name;
}

void ArgParser::AddHelp(const std::string& long_name, const std::string& description) {
    AddHelp(kNoShortName, long_name, description);
}

bool ArgParser::Help() const {
    return need_help_;
}

std::string ArgParser::HelpDescription() const {
    std::string result = program_name_ + '\n';

    size_t max_argument_names_length = 0;

    for (const Argument* argument : arguments_) {
        size_t length = GetArgumentNamesDescription(argument).length();
        if (length > max_argument_names_length) {
            max_argument_names_length = length;
        }
    }

    if (!program_description_.empty()) {
        result += program_description_;
        result += '\n';
    }

    result += "Usage: " + program_name_ + " [OPTIONS]";

    for (const Argument* argument : arguments_) {
        if (!argument->IsPositional()) {
            continue;
        }

        result += " <" + argument->GetLongName() + ">";

        if (argument->IsMultiValue()) {
            result += "...";
            break;
        }
    }

    result += "\nList of options:\n";

    for (const Argument* argument : arguments_) {
        if (argument->IsPositional()) {
            continue;
        }

        result += GetArgumentDescription(argument, max_argument_names_length);
        result += '\n';
    }

    return result;
}

std::string ArgParser::GetArgumentDescription(const Argument* argument,
                                              size_t max_argument_names_length) const {
    std::string result = GetArgumentNamesDescription(argument);

    result.insert(result.end(), max_argument_names_length - result.length() + 2, ' ');
    result += argument->GetDescription();

    std::string options = " [";

    bool is_first_option = true;

    if (argument->IsMultiValue()) {
        options += "repeated, min values = ";
        options += std::to_string(argument->GetMinimumValues());
        is_first_option = false;
    }

    if (argument->HasDefault() && argument->GetLongName() != help_argument_name_) {
        if (!is_first_option) {
            options += "; ";
        }

        options += "default = " + argument->GetDefaultValueString();
    }

    options += ']';

    if (options.length() > 3) {
        result += options;
    }

    return result;
}

std::string ArgParser::GetArgumentNamesDescription(const Argument* argument) const {
    std::string result;
    if (argument->GetShortName() == kNoShortName) {
        result.insert(0, 4, ' ');
    } else {
        result = "-";
        result += argument->GetShortName();
        result += ", ";
    }

    result += "--";
    result += argument->GetLongName();

    if (help_description_types_.contains(argument->GetType()) 
        && !help_description_types_.at(argument->GetType()).empty()) {
        result += "=<";
        result += help_description_types_.at(argument->GetType());
        result += ">";
    }

    return result;
}

ParsingError ArgParser::GetError() const {
    return error_;
}

bool ArgParser::HasError() const {
    return error_.status != ParsingErrorType::kSuccess;
}

std::optional<size_t> ArgParser::GetValuesSet(const std::string& long_name) const {
    if (!arguments_indeces_.contains(long_name)) {
        return std::nullopt;
    }

    return arguments_[arguments_indeces_.at(long_name)]->GetValuesSet();
}

std::optional<ArgumentStatus> ArgParser::GetValueStatus(const std::string& long_name) const {
    if (!arguments_indeces_.contains(long_name)) {
        return std::nullopt;
    }

    return arguments_[arguments_indeces_.at(long_name)]->GetValueStatus();
}

} // namespace ArgumentParser
