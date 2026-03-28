#include "capturedevice.hh"

CORBA::async<std::shared_ptr<ARKitFaceReceiver>> ARKitFaceDevice_impl::receiver() { co_return _receiver; }
CORBA::async<void> ARKitFaceDevice_impl::receiver(std::shared_ptr<ARKitFaceReceiver> receiver) {
    _receiver = receiver;
    co_return;
}

CORBA::async<std::shared_ptr<HolisticReceiver>> HolisticDevice_impl::receiver() { co_return _receiver; }
CORBA::async<void> HolisticDevice_impl::receiver(std::shared_ptr<HolisticReceiver> receiver) {
    _receiver = receiver;
    co_return;
}
