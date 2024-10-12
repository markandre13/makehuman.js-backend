#pragma once

#include <string>
#include <vector>
#include <cstdint>

uint64_t getMilliseconds();
std::vector<std::string_view> split(const std::string_view &data, char delimiter = '\n');
std::string_view trim(const std::string_view &data);

#define HAVE_FAST_FLOAT 1

#ifdef HAVE_FAST_FLOAT
#include "../upstream/fast_float/fast_float.h"
#endif

inline float stof(const std::string_view &s) {
#ifdef HAVE_FAST_FLOAT
    float value;
    fast_float::from_chars(s.data(), s.data() + s.size(), value);
    return value;
#else
    return strtof(s.data(), nullptr);
#endif
}

inline unsigned stou(const std::string_view &s) {
#ifdef HAVE_FAST_FLOAT
    unsigned value;
    fast_float::from_chars(s.data(), s.data() + s.size(), value);
    return value;
#else
    return strtoul(s.data(), nullptr, 10);
#endif
}
