// #include <uv.h>
// #include <libwebsockets.h>

// a single header file is required
#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>  // for puts
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <print>
#include <string>

#include "corba/corba.hh"
#include "corba/orb.hh"
#include "corba/protocol.hh"
#include "corba/net/ws.hh"

using namespace std;

class Backend {
};

class Backend_skel: public CORBA::Skeleton, public Backend {
public:
    Backend_skel(CORBA::ORB *orb) : Skeleton(orb) {}
    const char *repository_id() const override { return "IDL:Backend:1.0"; }
private:
    CORBA::async<> _call(const std::string &operation, CORBA::GIOPDecoder &decoder, CORBA::GIOPEncoder &encoder) override;
};

class Backend_impl : public Backend_skel {
    public:
        Backend_impl(CORBA::ORB *orb) : Backend_skel(orb) {}
};

CORBA::async<> Backend_skel::_call(const std::string &operation, CORBA::GIOPDecoder &decoder, CORBA::GIOPEncoder &encoder) { 
    println("Backend_impl::_call(\"{}\", decoder, encoder)", operation);
    encoder.ushort(4711);
    co_return;
}

int main(void) {
    printf("running\n");

    auto orb = make_shared<CORBA::ORB>();
    auto protocol = new MyProtocol();
    orb->registerProtocol(protocol);
    auto backend = make_shared<Backend_impl>(orb.get());
    orb->bind("Backend", backend);

    struct ev_loop *loop = EV_DEFAULT;
    protocol->listen(orb.get(), loop, "localhost", 9001);

    // initialise a timer watcher, then start it
    // simple non-repeating 5.5 second timeout
    // ev_timer_init(&timeout_watcher, timeout_cb, 5.5, 0.);
    // ev_timer_start(loop, &timeout_watcher);

    ev_run(loop, 0);

    return 0;
}
