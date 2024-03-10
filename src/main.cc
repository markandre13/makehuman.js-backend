#include <print>

#include <corba/corba.hh>
#include <corba/net/ws.hh>

#include "makehuman_impl.hh"

using namespace std;

int main(void) {
    println("makehuman.js backend");

    auto orb = make_shared<CORBA::ORB>();
    auto backend = make_shared<Backend_impl>(orb);
    orb->bind("Backend", backend);

    struct ev_loop *loop = EV_DEFAULT;
    auto protocol = new CORBA::net::WsProtocol();
    protocol->listen(orb.get(), loop, "localhost", 9001);

    orb->registerProtocol(protocol);

    println("the audience is listening...");
    ev_run(loop, 0);

    return 0;
}
