#pragma once

#include <string>
#include <vector>

std::vector<uint8_t> parseOmniDump(const std::string_view &data);
std::vector<std::string_view> split(std::string_view data);
std::string_view trim(std::string_view data);
void hexdump(unsigned char *buffer, int received);

