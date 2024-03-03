#pragma once

#include "../src/corba/hexdump.hh"
#include "../src/corba/coroutine.hh"
#include <functional>

#include <string>
#include <vector>

void parallel(std::exception_ptr &eptr, std::function<CORBA::async<>()> closure);
std::vector<uint8_t> parseOmniDump(const std::string_view &data);
std::vector<std::string_view> split(std::string_view data);
std::string_view trim(std::string_view data);

