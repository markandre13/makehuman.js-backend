#include "capturedevice.hh"

CORBA::async<std::shared_ptr<ARKitFaceReceiver>> ARKitFaceDevice_impl::receiver() { co_return _receiver; }
CORBA::async<void> ARKitFaceDevice_impl::receiver(std::shared_ptr<ARKitFaceReceiver> receiver) {
    _receiver = receiver;
    co_return;
}
