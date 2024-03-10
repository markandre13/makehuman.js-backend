#include "makehuman_impl.hh"
#include <print>

Backend_impl::Backend_impl(std::shared_ptr<CORBA::ORB> orb) : Backend_skel(orb) {}

CORBA::async<std::string> Backend_impl::hello(const std::string_view &word) {
    std::println("Backend_impl::hello(\"{}\") -> \"{} world.\"", word, word);
    co_return std::string(word) + " world.";
}

CORBA::async<> Backend_impl::fail() {
    throw CORBA::BAD_OPERATION(0, CORBA::YES);
    co_return;
}