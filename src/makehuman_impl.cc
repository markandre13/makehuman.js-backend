#include "makehuman_impl.hh"
#include <print>

Backend_impl::Backend_impl(std::shared_ptr<CORBA::ORB> orb) : Backend_skel(orb) {}

CORBA::async<> Backend_impl::setFrontend(std::shared_ptr<Frontend> aFrontend) {
    frontend = aFrontend;
    co_await frontend->hello();
    co_return;
}

CORBA::async<> Backend_impl::setEngine(MotionCaptureEngine engine, MotionCaptureType type, EngineStatus status) {
    std::println("SET ENGINE engine, type, status");
    co_return; 
}
