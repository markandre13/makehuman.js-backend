#pragma once

#include <ev.h>
#include <functional>

/**
 * Execute the user provided callback via libev
 */
class Async {
        ev_async watcher;
        struct ev_loop *_loop;
        std::function<void()> cb;

        static void libev_async_cb(struct ev_loop *loop, struct ev_async *watcher, int revents);

    public:
        Async &operator=(const Async &) = delete;
        Async(const Async &) = delete;
        Async(const Async &&) = delete;

        Async(struct ev_loop *loop, std::function<void()> cb);
        ~Async();

        void emit();
};
