#include "async.hh"

#include <print>

Async::Async(struct ev_loop *loop, std::function<void()> cb) : _loop(loop), cb(cb) {
    ev_async_init(&watcher, libev_async_cb);
    ev_async_start(_loop, &watcher);
}
Async::~Async() {
    ev_async_stop(_loop, &watcher);
}

void Async::emit() {
    ev_async_send(_loop, &watcher);
}

void Async::libev_async_cb(struct ev_loop *loop, struct ev_async *watcher, int revents) {
    auto async = reinterpret_cast<Async *>(watcher);
    async->cb();
}