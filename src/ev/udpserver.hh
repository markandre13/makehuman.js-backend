#pragma once

#include <ev.h>

class UDPServer {
        static void libev_read_cb(struct ev_loop *loop, struct ev_io *watcher, int revents);
        struct ev_loop *loop;
        unsigned port;
        struct handler_t;
        handler_t *handler;
    protected:
        int fd;
    public:
        UDPServer(struct ev_loop *loop, unsigned port);
        virtual ~UDPServer();
        virtual void read() = 0;
};
