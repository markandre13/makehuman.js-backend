#pragma once

#include <ev.h>

#include <functional>

class Timer {
        ev_timer watcher;
        struct ev_loop *loop;
        std::function<void()> cb;

        static void libev_timer_cb(struct ev_loop *loop, struct ev_timer *watcher, int revents);

    public:
        Timer &operator=(const Timer &) = delete;
        Timer(const Timer &) = delete;
        Timer(const Timer &&) = delete;

        Timer(struct ev_loop *loop, ev_tstamp initialDelay, ev_tstamp interval, std::function<void()> cb);
        ~Timer();
        void stop();
};