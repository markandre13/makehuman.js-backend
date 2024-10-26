#pragma once

#include <ev.h>

class Timer {
        ev_timer watcher;
        struct ev_loop *loop;
        std::function<void()> cb;

        static void libev_timer_cb(struct ev_loop *loop, struct ev_timer *watcher, int revents) {
            auto timer = reinterpret_cast<Timer *>(watcher);
            timer->cb();
        }

    public:
        Timer(struct ev_loop *loop, ev_tstamp initialDelay, ev_tstamp interval, std::function<void()> cb) : loop(loop), cb(cb) {
            ev_timer_init(&watcher, libev_timer_cb, initialDelay, interval);
            ev_timer_start(loop, &watcher);
        }
        ~Timer() { stop(); }

        void stop() { ev_timer_stop(loop, &watcher); }
};