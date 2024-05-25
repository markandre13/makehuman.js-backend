#pragma once

#include <functional>
#include "udpserver.hh"

class LiveLinkFrame;

/**
 * Epic Games / Unreal Engine Live Link Face
 */
class LiveLink: UDPServer {
    public: 
        std::function<void(const LiveLinkFrame&)> callback;
        LiveLink(struct ev_loop *loop, unsigned port, std::function<void(const LiveLinkFrame&)>);
        void read() override;
};
