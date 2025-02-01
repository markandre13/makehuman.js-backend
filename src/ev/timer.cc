#include "timer.hh"

// #include <print>

Timer::Timer(struct ev_loop *loop, ev_tstamp initialDelay, ev_tstamp interval, std::function<void()> cb) : loop(loop), cb(cb) {
    // std::println("Timer::Timer(loop, initialDelay={}, interval={}, callback)", initialDelay, interval);
    ev_timer_init(&watcher, libev_timer_cb, initialDelay, interval);
    ev_timer_start(loop, &watcher);
    // std::println("timer watcher is active: {}", ev_is_active(&watcher));
}
Timer::~Timer() {
    // std::println("Timer::~Timer()");
    stop();
}

void Timer::stop() {
    // std::println("Timer::stop()");
    ev_timer_stop(loop, &watcher);
}

void Timer::libev_timer_cb(struct ev_loop *loop, struct ev_timer *watcher, int revents) {
    // std::println("Timer::libev_timer_cb(...)");
    auto timer = reinterpret_cast<Timer *>(watcher);
    timer->cb();
}
