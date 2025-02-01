#include "../src/ev/timer.hh"

#include <thread>

#include "kaffeeklatsch.hh"

using namespace kaffeeklatsch;
using namespace std;

class MoCapPlayer2 {
    private:
        Timer timer;

    public:
        MoCapPlayer2(struct ev_loop *loop);
        ~MoCapPlayer2();
    private:
        void tick();
};

MoCapPlayer2::MoCapPlayer2(struct ev_loop *loop)
    : timer(loop, 0.0, 1.0 / 30.0,
            [this] {
                this->tick();
            })
{
    println("MoCapPlayer2::MoCapPlayer2()");
}

MoCapPlayer2::~MoCapPlayer2() {
    println("MoCapPlayer2::~MoCapPlayer()");
}

void MoCapPlayer2::tick() {
    println("MoCapPlayer2::tick()");
}

kaffeeklatsch_spec([] {
    describe("class Timer", [] {
        fit("Timer(loop, delay, interval, closure)", [] {
            struct ev_loop *loop = EV_DEFAULT;

            auto endTimer = make_unique<Timer>(loop, 0.1, 0, [&] {
                println("done");
                ev_break(loop);
            });

            println("enter libev thread");
            std::thread libevthread(ev_run, loop, 0);

            unsigned counter = 0;
            auto pulseTimer = make_unique<Timer>(loop, 0, 0.01, [&] {
                println("tick");
                ++counter;
            });

            libevthread.join();
            println("left libev thread");

            expect(counter).to.be.greaterThanOrEqual(10);
            expect(counter).to.be.lessThanOrEqual(11);
        });

        fit("", [] {
            struct ev_loop *loop = EV_DEFAULT;
            auto player = make_shared<MoCapPlayer2>(loop);
            ev_run(loop, 0);
        });
    });
});
