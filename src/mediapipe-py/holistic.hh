#pragma once

#include "../capturedevice.hh"
#include "../ev/udpserver.hh"

/**
 * Receive ARKit holistic (face, post, left and right hand) data via UDP from Python Mediapipe job
 */
class MediapipePyHolisticDevice : public virtual HolisticDevice_impl, private UDPServer {
    public:
        MediapipePyHolisticDevice(struct ev_loop* loop, unsigned port);
        std::string id() override;
        CaptureDeviceType type() override;
        std::string name() override;

    private:
        void read() override;
};
