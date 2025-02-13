#pragma once

#include "Argument.hpp"
#include "utils/utils.hpp"

#include <cstddef>
#include <type_traits>
#include <expected>
#include <sstream>

namespace ArgumentParser {

template<typename T>
std::optional<T> ParseValue(std::string_view value_string);

template<typename T>
class SpecificArgument : public Argument {
public:
    SpecificArgument() = delete;
    SpecificArgument(char short_name,
                     const std::string& long_name,
                     const std::string& description);

    ~SpecificArgument();
    SpecificArgument(const SpecificArgument&) = delete;
    SpecificArgument& operator=(const SpecificArgument&) = delete;

    std::string_view GetType() const override;
    ArgumentStatus GetValueStatus() const override;
    size_t GetValuesSet() const override;

    std::expected<size_t, ParsingError> ParseArgument(const std::vector<std::string_view>& argv,
                                                      size_t position) override;

    std::optional<T> GetValue(size_t index = 0) const;

    SpecificArgument& Default(T default_value);
    SpecificArgument& MultiValue(size_t min_values = 0);
    SpecificArgument& Positional();
    SpecificArgument& StoreValue(T& to);
    SpecificArgument& StoreValues(std::vector<T>& to);

    void Clear() override;

    const std::string& GetDefaultValueString() const override;
    void SetDefaultValueString(const std::string& str) override;

    const std::string& GetDescription() const override;
    const std::string& GetLongName() const override;
    char GetShortName() const override;
    bool IsPositional() const override;
    bool IsMultiValue() const override;
    bool HasDefault() const override;
    size_t GetMinimumValues() const override;

    bool IsFlag() const override;

protected:
    std::string long_name_;
    char short_name_ = kNoShortName;
    std::string description_;

    ArgumentStatus value_status_ = ArgumentStatus::kNoArgument;

    T value_;
    T default_value_{};
    std::string default_value_string_;
    bool was_default_value_string_set_ = false;

    T* store_value_to_ = nullptr;
    std::vector<T>* store_values_to_ = nullptr;

    bool was_temp_vector_created_ = false;

    size_t minimum_values_ = 0;
    bool is_multi_value_ = false;
    bool is_positional_ = false;
    bool has_default_ = false;
    bool has_store_values_ = false;
    bool has_store_value_ = false;

    bool is_flag_ = false;

    size_t values_set_ = 0;
};

template<typename T>
SpecificArgument<T>::SpecificArgument(char short_name,
                                      const std::string& long_name,
                                      const std::string& description) 
    : short_name_(short_name),
      long_name_(long_name),
      description_(description) {
    if (std::is_same_v<bool, T>) {
        default_value_string_ = "false";
        has_default_ = true;
        is_flag_ = true;
    }

    store_values_to_ = new std::vector<T>;
    was_temp_vector_created_ = true;
}

template <typename T>
std::expected<size_t, ParsingError> SpecificArgument<T>::ParseArgument(
    const std::vector<std::string_view>& argv,
    size_t position) {
    if (store_values_to_->empty()) {
        value_status_ = ArgumentStatus::kSuccess;
    }

    size_t current_used_positions = 1;
    
    std::string_view value_string = argv[position];

    bool is_short_with_value = false;

    if (!is_positional_) {
        if (value_string.starts_with("--")) {
            value_string = value_string.substr(2);
        } else if (value_string[0] == '-') {
            value_string = value_string.substr(1);

            if (value_string.length() > 1 && value_string[1] != '=') {
                value_string = value_string.substr(1);
                is_short_with_value = true;
            }
        }
    }

    size_t equal_sign_index = value_string.find('=');

    if (!is_positional_ && equal_sign_index != std::string_view::npos) {
        value_string = value_string.substr(equal_sign_index + 1);
    } else if (std::is_same_v<bool, T>) {
        value_string = "";
    } else if (!is_positional_ && !is_short_with_value) {
        if (position == argv.size() - 1) {
            value_status_ = ArgumentStatus::kInsufficient;
            return std::unexpected(ParsingError{argv[position], ParsingErrorType::kInsufficent, long_name_});
        }

        ++position;
        ++current_used_positions;
        value_string = argv[position];
    }

    auto parsing_result = ParseValue<T>(value_string);

    if (!parsing_result.has_value()) {
        value_status_ = ArgumentStatus::kInvalidArgument;
        return std::unexpected(ParsingError{argv[position], ParsingErrorType::kInvalidArgument, long_name_});
    }

    value_ = parsing_result.value();

    store_values_to_->push_back(value_);
    ++values_set_;

    if (has_store_value_) {
        *store_value_to_ = value_;
    }

    if (values_set_ < minimum_values_ && !has_default_) {
        value_status_ = ArgumentStatus::kInsufficient;
    } else {
        value_status_ = ArgumentStatus::kSuccess;
    }

    return current_used_positions;
}

template <typename T>
SpecificArgument<T>::~SpecificArgument() {
    if (was_temp_vector_created_) {
        delete store_values_to_;
    }
}

template<typename T>
std::optional<T> SpecificArgument<T>::GetValue(size_t index) const {
    if (is_multi_value_ && has_default_ && index >= store_values_to_->size()) {
        return default_value_;
    }

    if (store_values_to_->size() == 0 && has_default_) {
        return default_value_;
    }

    if (index >= store_values_to_->size()) {
        return std::nullopt;
    }

    return store_values_to_->at(index);
}

template <typename T>
void SpecificArgument<T>::Clear() {
    store_values_to_->clear();
    values_set_ = 0;

    value_status_ = has_default_ ? ArgumentStatus::kSuccess : ArgumentStatus::kNoArgument;

    if (has_store_value_) {
        *store_value_to_ = default_value_;
    }
}

template<typename T>
SpecificArgument<T>& SpecificArgument<T>::Default(T default_value) {
    if (!was_default_value_string_set_) {
        std::ostringstream stream;
        stream << default_value;
        default_value_string_ = stream.str();
        
        if (std::is_same_v<bool, T>) {
            default_value_string_ = (default_value_string_ == "1") ? "true" : "false";
        }
    }

    default_value_ = default_value;
    has_default_ = true;
    value_status_ = ArgumentStatus::kSuccess;
    return *this;
}

template<typename T>
SpecificArgument<T>& SpecificArgument<T>::MultiValue(size_t min_values) {
    minimum_values_ = min_values;
    is_multi_value_ = true;

    return *this;
}

template<typename T>
SpecificArgument<T>& SpecificArgument<T>::Positional() {
    is_positional_ = true;
    return *this;
}

template<typename T>
SpecificArgument<T>& SpecificArgument<T>::StoreValue(T& to) {
    store_value_to_ = &to;
    has_store_value_ = true;
    return *this;
}

template<typename T>
SpecificArgument<T>& SpecificArgument<T>::StoreValues(std::vector<T>& to) {
    if (was_temp_vector_created_) {
        delete store_values_to_;
        was_temp_vector_created_ = false;
    }

    store_values_to_ = &to;
    has_store_values_ = true;
    return *this;
}

template <typename T>
size_t SpecificArgument<T>::GetValuesSet() const {
    return values_set_;
}

template<typename T>
std::string_view SpecificArgument<T>::GetType() const {
    return typeid(T).name();
}

template <typename T>
ArgumentStatus SpecificArgument<T>::GetValueStatus() const {
    return value_status_;
}

template <typename T>
const std::string& SpecificArgument<T>::GetDefaultValueString() const {
    return default_value_string_;
}

template <typename T>
void SpecificArgument<T>::SetDefaultValueString(const std::string& str) {
    default_value_string_ = str;
    was_default_value_string_set_ = true;
}

template <typename T>
const std::string& SpecificArgument<T>::GetDescription() const {
    return description_;
}

template <typename T>
const std::string& SpecificArgument<T>::GetLongName() const {
    return long_name_;
}

template <typename T>
char SpecificArgument<T>::GetShortName() const {
    return short_name_;
}

template <typename T>
bool SpecificArgument<T>::IsPositional() const {
    return is_positional_;
}

template <typename T>
bool SpecificArgument<T>::IsMultiValue() const {
    return is_multi_value_;
}

template <typename T>
bool SpecificArgument<T>::HasDefault() const {
    return has_default_;
}

template <typename T>
size_t SpecificArgument<T>::GetMinimumValues() const {
    return minimum_values_;
}

template <typename T>
bool SpecificArgument<T>::IsFlag() const {
    return is_flag_;
}

} // namespace ArgumentParser
