#pragma once

#include <string>
#include <cstddef>

void _hexdump(const unsigned char *buffer, std::size_t nbytes);
inline void hexdump(const unsigned char *buffer, std::size_t nbytes) { _hexdump(buffer, nbytes); }
// inline void hexdump(const uint8_t *buffer, size_t nbytes) { _hexdump(reinterpret_cast<const unsigned char *>(buffer), nbytes); }
inline void hexdump(const char *buffer, std::size_t nbytes) { _hexdump(reinterpret_cast<const unsigned char *>(buffer), nbytes); }
inline void hexdump(const void *buffer, std::size_t nbytes) { _hexdump(reinterpret_cast<const unsigned char *>(buffer), nbytes); }
inline void hexdump(const std::string &str) { _hexdump(reinterpret_cast<const unsigned char *>(str.data()), str.size()); }
