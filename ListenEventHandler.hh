#pragma once
#include "EventHandler.hh"

class ListenEventHandler : public EventHandler {
    public:
        ListenEventHandler(int fd) : fd_(fd), cfd_(-1) {}
        virtual ~ListenEventHandler();
        virtual int on_read_event();
        virtual int on_write_event() { return 0; }
        virtual bool want_read() { return true; }
        virtual bool want_write() { return false; }
        virtual int fd() const { return fd_; }
        virtual const char *name() const { return "ListenEventHandler"; }
        virtual bool finish() { return false; }
        virtual EventHandler *next();

    private:
        int fd_;
        int cfd_;
};