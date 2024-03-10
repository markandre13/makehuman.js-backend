#include <corba/corba.hh>
#include <corba/net/ws.hh>

using namespace std;

int main(void) {
    printf("running\n");

    // auto orb = make_shared<CORBA::ORB>();
    // auto protocol = new WsProtocol();
    // orb->registerProtocol(protocol);
    // auto backend = make_shared<Backend_impl>(orb);
    // orb->bind("Backend", backend);

    // struct ev_loop *loop = EV_DEFAULT;
    // protocol->listen(orb.get(), loop, "localhost", 9001);

    // // initialise a timer watcher, then start it
    // // simple non-repeating 5.5 second timeout
    // // ev_timer_init(&timeout_watcher, timeout_cb, 5.5, 0.);
    // // ev_timer_start(loop, &timeout_watcher);

    // ev_run(loop, 0);

    return 0;
}
