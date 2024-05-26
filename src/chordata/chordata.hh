#pragma once

#include <functional>
#include "../ev/udpserver.hh"

/**
 * Chordata Motion COOP/OSC listener
 */
class Chordata: UDPServer {
        std::function<void(const char *buffer, size_t nbytes)> callback;
    public: 
        Chordata(struct ev_loop *loop, unsigned port, std::function<void(const char *buffer, size_t nbytes)> callback);
    protected:
        void read() override;
};
