#pragma once

#include "generated/makehuman_skel.hh"

class LocalCaptureDevice : public virtual CaptureDevice {
    public:
        virtual std::string id() = 0;
        virtual CaptureDeviceType type() = 0;
        virtual std::string name() = 0;
};

class ARKitFaceDevice_impl : public virtual ARKitFaceDevice_skel, public virtual LocalCaptureDevice {
    protected:
        std::shared_ptr<ARKitFaceReceiver> _receiver;

    public:
        CORBA::async<std::shared_ptr<ARKitFaceReceiver>> receiver() override;
        CORBA::async<void> receiver(std::shared_ptr<ARKitFaceReceiver>) override;
};
