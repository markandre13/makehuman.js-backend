#include "../src/corba/net/ws.hh"

#include "../../makehuman_impl.hh"
#include "../../makehuman_stub.hh"
#include "../../util.hh"
#include "../src/corba/corba.hh"
#include "kaffeeklatsch.hh"

// import std;

using namespace kaffeeklatsch;
using namespace std;
using CORBA::async;

kaffeeklatsch_spec([] {
    describe("net", [] {
        describe("websocket", [] {
            xit("do it", [] {
                struct ev_loop *loop = EV_DEFAULT;

                // start server & client on the same ev loop
                auto serverORB = make_shared<CORBA::ORB>();
                auto protocol = new CORBA::net::WsProtocol();
                serverORB->registerProtocol(protocol);
                protocol->listen(serverORB.get(), loop, "localhost", 9001);

                auto backend = make_shared<Backend_impl>(serverORB);
                serverORB->bind("Backend", backend);

                std::exception_ptr eptr;

                parallel(eptr, [loop] -> async<> {
                    auto clientORB = make_shared<CORBA::ORB>();
                    auto protocol = new CORBA::net::WsProtocol();
                    clientORB->registerProtocol(protocol);
                    protocol->attach(clientORB.get(), loop);
                    auto object = co_await clientORB->stringToObject("corbaname::localhost:9001#Backend");
                    auto backend = Backend::_narrow(object);
                });


                // initialise a timer watcher, then start it
                // simple non-repeating 5.5 second timeout
                // ev_timer_init(&timeout_watcher, timeout_cb, 5.5, 0.);
                // ev_timer_start(loop, &timeout_watcher);

                // ev_break

                ev_run(loop, 0);

                if (eptr) {
                    std::rethrow_exception(eptr);
                }
            });
        });
    });
});