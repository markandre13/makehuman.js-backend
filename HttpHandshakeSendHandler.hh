#pragma once

#include "EventHandler.hh"

#include <string>

class HttpHandshakeSendHandler : public EventHandler
{
public:
    HttpHandshakeSendHandler(int fd, const std::string &accept_key);
    virtual ~HttpHandshakeSendHandler();
    virtual int on_read_event() { return 0; }
    virtual int on_write_event();
    virtual bool want_read() { return false; }
    virtual bool want_write() { return true; }
    virtual int fd() const { return fd_; }
    virtual const char *name() const { return "HttpHandshakeSendHandler"; }
    virtual bool finish() { return off_ == resheaders_.size(); }
    virtual EventHandler *next();
private:
    int fd_;
    std::string headers_;
    std::string resheaders_;
    size_t off_;
};