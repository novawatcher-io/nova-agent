//
// Created by root on 2024/4/1.
//

#pragma once

#include <absl/numeric/int128.h>
#include <utility>

#include "opentelemetry/nostd/span.h"
namespace App {
namespace Common {
//* 将数字转为16进制（大写）
inline char ToHexUpper(unsigned int value) {
    return "0123456789ABCDEF"[value & 0xF];
}

//* 将数字转为16进制（小写）
inline char ToHexLower(unsigned int value) {
    return "0123456789abcdef"[value & 0xF];
}

//* 将数16进（大写或小写）制转为数字
inline int FromHex(unsigned int c) {
    return ((c >= '0') && (c <= '9'))   ? int(c - '0')
           : ((c >= 'A') && (c <= 'F')) ? int(c - 'A' + 10)
           : ((c >= 'a') && (c <= 'f')) ? int(c - 'a' + 10)
                                        :
                                        /* otherwise */ -1;
}

inline static __int128_t atoint128_t(const char *s)
{
    const char *p = s;
    __int128_t val = 0;

    if (*p == '-' || *p == '+') {
        p++;
    }
    while (*p >= '0' && *p <= '9') {
        val = (10 * val) + (*p - '0');
        p++;
    }
    if (*s == '-') val = val * -1;
    return val;
}

inline static std::pair<absl::uint128, bool> isUint128(const std::string& str) {
   if (str.empty()) {
    return {0, false};
  }
   if (str.length() > 40) {
       return {0, false};
   }
  absl::uint128 result = 0;
  for (char c : str) {
    if (!std::isdigit(c)) {
      return {0, false};
    }
    result *= 10;
    result += c - '0';
  }
  return {result, true};
}

inline static __int64_t atoi64_t(const char *s)
{
    const char *p = s;
    __int64_t val = 0;

    if (*p == '-' || *p == '+') {
        p++;
    }
    while (*p >= '0' && *p <= '9') {
        val = (10 * val) + (*p - '0');
        p++;
    }
    if (*s == '-') val = val * -1;
    return val;
}

//* 将数据d用16进制解码，返回值即是结果
inline static opentelemetry::nostd::span<uint8_t, 16> HexDecode(const std::string& hex) {
    opentelemetry::nostd::span<uint8_t, 16> res(nullptr, 16);
    unsigned char* pResult = (unsigned char*)res.data() + res.size();
    bool odd_digit = true;

    for (int i = hex.size() - 1; i >= 0; i--) {
        auto ch = uint8_t(hex.at(i));
        int tmp = FromHex(ch);
        if (tmp == -1) {
            continue;
        }
        if (odd_digit) {
            --pResult;
            *pResult = tmp;
            odd_digit = false;
        } else {
            *pResult |= tmp << 4;
            odd_digit = true;
        }
    }
    return res;
}

// 将十六进制字符转换为对应的十进制数字
static unsigned char chex_fromxdigit(unsigned h) {
    return ((h & 0xf) + (h >> 6) * 9);
}

inline static void HexDecode(const std::string& hex, opentelemetry::nostd::span<uint8_t, 16>& res) {
    int count = 0;
    for (unsigned i = 0; i < hex.length(); i += 2) {
        unsigned char hi = chex_fromxdigit(hex[i]);
        unsigned char lo = chex_fromxdigit(hex[i + 1]);
        res[count] = ((hi << 4) | lo);
        count++;
    }
}

} // namespace Common
} // namespace App
