#include "../src/ev/async.hh"
#include "../src/ev/timer.hh"

#include <thread>

#include "kaffeeklatsch.hh"

using namespace kaffeeklatsch;
using namespace std;

kaffeeklatsch_spec([] {
    describe("class Async", [] {
        it("Async(loop, cb)", [] {
            struct ev_loop *loop = EV_DEFAULT;
            auto async = make_shared<Async>(loop, [&]{
                ev_break(loop);
            });
            
            auto pulseTimer = make_unique<Timer>(loop, 0, 1.0, [&] {
                async->emit();
            });

            std::thread libevthread(ev_run, loop, 0);

            libevthread.join();
        });
    });
});
