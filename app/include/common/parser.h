#pragma once
#include <absl/strings/numbers.h>
#include <cstdlib>
#include <spdlog/spdlog.h>
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>

namespace App::Common {

template <typename T> //
bool ConvertStr2Number(std::string_view str, T* val, bool simple) {
    if constexpr (std::is_integral_v<T>) {
        if (simple) {
            return absl::SimpleAtoi(str, val);
        } else {
            char* end = nullptr;
            long result = strtol(str.data(), &end, 10);
            if (end != str.data()) {
                *val = static_cast<T>(result);
                return true;
            }
        }
        SPDLOG_ERROR("parse integer failed, value={}", str);
        return false;
    } else if constexpr (std::is_same_v<T, double>) {
        if (simple) {
            return absl::SimpleAtod(str, val);
        } else {
            char* end = nullptr;
            const double result = strtod(str.data(), &end);
            if (end != str.data()) {
                *val = result;
                return true;
            }
        }
        SPDLOG_ERROR("parse double failed, value={}", str);
        return false;
    } else if constexpr (std::is_same_v<T, float>) {
        if (simple) {
            return absl::SimpleAtof(str, val);
        } else {
            char* end = nullptr;
            double const result = strtod(str.data(), &end);
            if (end != str.data()) {
                *val = static_cast<float>(result);
                return true;
            }
        }
        SPDLOG_ERROR("parse float failed, value={}", str);
        return false;
    } else {
        static_assert(false, "ValueType must be float number or integral type");
    }
}

template <typename... ValueTypes>
bool ParseStr2Number(std::string_view str, std::variant<ValueTypes...> val, bool simple) {
    return std::visit([&str, simple](auto arg) -> bool { return ConvertStr2Number(str, arg, simple); }, val);
}

template <typename... ValueType> //
bool ParseKeyVal(std::string_view content, std::string_view key, std::variant<ValueType...> val, bool simple) {
    auto pos = content.find(key);
    if (pos == std::string::npos) {
        return false;
    }
    pos += key.size();
    while (pos < content.size() && (content[pos] == ' ' || content[pos] == '\t')) {
        pos++;
    }
    auto end = content.find('\n', pos);
    if (end == std::string::npos) {
        return false;
    }
    auto value = content.substr(pos, end - pos);
    return std::visit(
        [&value, simple](auto arg) -> bool {
            using UnderlyingType = std::remove_pointer_t<decltype(arg)>;
            if constexpr (std::is_same_v<UnderlyingType, std::string>) {
                *arg = value;
                return true;
            } else if constexpr (std::is_integral_v<UnderlyingType> || std::is_floating_point_v<UnderlyingType>) {
                return ConvertStr2Number(value, arg, simple);
            } else {
                static_assert(false, "ValueType must be one of std::string/integer/floating");
            }
        },
        val);
}

template <typename ValueType> //
bool ReadKeyVal(std::string_view content, std::string_view key, ValueType* val, bool simple, bool required = true) {
    if (val == nullptr) {
        return false;
    }

    auto pos = content.find(key);
    if (pos == std::string::npos) {
        return !required;
    }
    pos += key.size();
    while (pos < content.size() && (content[pos] == ' ' || content[pos] == '\t')) {
        pos++;
    }
    auto end = content.find('\n', pos);
    std::string_view value;
    if (end == std::string::npos) {
        value = content.substr(pos);
    } else {
        value = content.substr(pos, end - pos);
    }

    if constexpr (std::is_same_v<ValueType, std::string>) {
        *val = value;
        return true;
    } else if constexpr (std::is_integral_v<ValueType> || std::is_floating_point_v<ValueType>) {
        return ConvertStr2Number(value, val, simple);
    } else {
        static_assert(false, "ValueType must be std::string or integral type");
    }
}

} // namespace App::Common