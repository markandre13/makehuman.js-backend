#pragma once

#include "makehuman_skel.hh"
#include <span>

class Backend_impl : public Backend_skel {
        std::shared_ptr<Frontend> frontend;
    public:
        Backend_impl(std::shared_ptr<CORBA::ORB> orb);
        CORBA::async<> setFrontend(std::shared_ptr<Frontend> frontend) override;
        CORBA::async<> setEngine(MotionCaptureEngine engine, MotionCaptureType type, EngineStatus status) override;

        inline void mediapipe(std::span<float> &data) {
            if (frontend) {
                frontend->mediapipe(data);
            }
        }
};

// this would only be needed for testing

// class Frontend_impl : public Frontend_skel {
//     public:
//         Frontend_impl(std::shared_ptr<CORBA::ORB> orb);
//         CORBA::async<void> hello() override;
// };
