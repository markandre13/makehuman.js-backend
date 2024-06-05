#pragma once

#include <functional>
#include "../ev/udpserver.hh"

class CaptureEngine {
    public:
        virtual ~CaptureEngine();
};

class LiveLinkFrame;

/**
 * Epic Games / Unreal Engine Live Link Face
 */
class LiveLink: UDPServer, public CaptureEngine {
        std::function<void(const LiveLinkFrame&)> callback;
    public: 
        LiveLink(struct ev_loop *loop, unsigned port, std::function<void(const LiveLinkFrame&)>);
        virtual ~LiveLink();
    protected:
        void read() override;
};
