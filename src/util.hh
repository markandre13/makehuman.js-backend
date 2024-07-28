#pragma once

#include <string>
#include <vector>
#include <cstdint>

uint64_t getMilliseconds();
std::vector<std::string_view> split(const std::string_view &data, char delimiter = '\n');
std::string_view trim(const std::string_view &data);
