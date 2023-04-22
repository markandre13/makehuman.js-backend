#pragma once
#include "EventHandler.hh"
#include <string>

class HttpHandshakeRecvHandler : public EventHandler
{
public:
    HttpHandshakeRecvHandler(int fd) : fd_(fd) {}
    virtual ~HttpHandshakeRecvHandler();
    virtual int on_read_event();
    virtual int on_write_event() { return 0; }
    virtual bool want_read() { return true; }
    virtual bool want_write() { return false; }
    virtual int fd() const { return fd_; }
    virtual const char *name() const { return "HttpHandshakeRecvHandler"; }
    virtual bool finish() { return !accept_key_.empty(); }
    virtual EventHandler *next();

private:
    int fd_;
    std::string headers_;
    std::string accept_key_;
};
