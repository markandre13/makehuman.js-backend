#pragma once

class EventHandler {
    public:
        virtual ~EventHandler() {}
        virtual int on_read_event() = 0;
        virtual int on_write_event() = 0;
        virtual bool want_read() = 0;
        virtual bool want_write() = 0;
        virtual int fd() const = 0;
        virtual const char *name() const = 0;
        virtual bool finish() = 0;
        virtual EventHandler *next() = 0;
};

void wsInit();
void wsHandle();

// void reactor(EventHandler *listen_handler);
