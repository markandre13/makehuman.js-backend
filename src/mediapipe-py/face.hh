#pragma once

#include "../capturedevice.hh"
#include "../ev/udpserver.hh"

/**
 * Receive ARKit face data via UDP from Python Mediapipe job
 */
class MediapipePyFaceDevice : public virtual ARKitFaceDevice_impl, private UDPServer {
    public:
        MediapipePyFaceDevice(struct ev_loop *loop, unsigned port);
        CORBA::async<void> receiver(std::shared_ptr<ARKitFaceReceiver>) override;
        std::string id() override;
        CaptureDeviceType type() override;
        std::string name() override;

    private:
        bool _blendshapeNamesHaveBeenSend;
        void read() override;
};