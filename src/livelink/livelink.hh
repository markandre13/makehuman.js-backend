#pragma once

#include <functional>
#include "../ev/udpserver.hh"

class LiveLinkFrame;

/**
 * Epic Games / Unreal Engine Live Link Face
 */
class LiveLink: UDPServer {
        std::function<void(const LiveLinkFrame&)> callback;
    public: 
        LiveLink(struct ev_loop *loop, unsigned port, std::function<void(const LiveLinkFrame&)>);
    protected:
        void read() override;
};
