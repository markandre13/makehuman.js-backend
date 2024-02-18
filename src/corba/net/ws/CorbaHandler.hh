#pragma once

#include <wslay/wslay.h>
#include "EventHandler.hh"

bool isChordataRequested();
void sendChordata(void* data, size_t size);

class CorbaHandler : public EventHandler {
    public:
        CorbaHandler(int fd);
        virtual ~CorbaHandler();
        virtual int on_read_event() { return wslay_event_recv(ctx_) == 0 ? 0 : -1; }
        virtual int on_write_event() { return wslay_event_send(ctx_) == 0 ? 0 : -1; }

        ssize_t send_data(const uint8_t *data, size_t len, int flags);
        ssize_t recv_data(uint8_t *data, size_t len, int flags);

        virtual bool want_read() { return wslay_event_want_read(ctx_); }
        virtual bool want_write() { return wslay_event_want_write(ctx_); }
        virtual int fd() const { return fd_; }
        virtual const char *name() const { return "CorbaHandler"; }
        virtual bool finish() { return !want_read() && !want_write(); }
        virtual EventHandler *next() { return 0; }

    // private:
        int fd_;
        wslay_event_context_ptr ctx_;
};
