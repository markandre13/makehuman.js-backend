#pragma once

#include "../capturedevice.hh"
#include "../ev/udpserver.hh"

/**
 * Receive ARKit face data via UDP from Python Mediapipe job
 */
class MediapipePyFaceDevice : public virtual ARKitFaceDevice_impl, private UDPServer {
    public:
        MediapipePyFaceDevice(struct ev_loop* loop, unsigned port);
        std::string id() override;
        CaptureDeviceType type() override;
        std::string name() override;

    private:
        void read() override;
};