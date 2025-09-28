#pragma once

#include "../capturedevice.hh"
#include "../ev/udpserver.hh"

/**
 * Epic Games / Unreal Engine Live Link Face
 */
class LiveLinkFaceDevice : public virtual ARKitFaceDevice_impl, private UDPServer {
    public:
        LiveLinkFaceDevice(struct ev_loop *loop, unsigned port);
        CORBA::async<void> receiver(std::shared_ptr<ARKitFaceReceiver>) override;
        virtual CORBA::async<CaptureDeviceType> type() override;
        virtual CORBA::async<std::string> name() override;

    private:
        bool _blendshapeNamesHaveBeenSend;
        void read() override;
};