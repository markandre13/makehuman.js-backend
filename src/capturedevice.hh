#include "generated/makehuman_skel.hh"

class ARKitFaceDevice_impl : public virtual ARKitFaceDevice_skel {
    protected:
        std::shared_ptr<ARKitFaceReceiver> _receiver;

    public:
        CORBA::async<std::shared_ptr<ARKitFaceReceiver>> receiver() override;
        CORBA::async<void> receiver(std::shared_ptr<ARKitFaceReceiver>) override;
};
