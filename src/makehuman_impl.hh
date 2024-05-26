#pragma once

#include "makehuman_skel.hh"
#include <span>

#ifdef HAVE_MEDIAPIPE
#include <cc_lib/mediapipe.hh>
#endif

class Backend_impl : public Backend_skel {
        // std::atomic<std::shared_ptr<Frontend>> frontend;
        std::shared_ptr<Frontend> frontend;
        bool blendshapeNamesHaveBeenSend = false;
    public:
        Backend_impl(std::shared_ptr<CORBA::ORB> orb);
        CORBA::async<> setFrontend(std::shared_ptr<Frontend> frontend) override;
        CORBA::async<> setEngine(MotionCaptureEngine engine, MotionCaptureType type, EngineStatus status) override;

        void chordata(const char *buffer, size_t nbytes);

#ifdef HAVE_MEDIAPIPE
        void faceLandmarks(
            std::optional<mediapipe::cc_lib::vision::face_landmarker::FaceLandmarkerResult> result,
            int64_t timestamp_ms
        );
#endif
};

// this would only be needed for testing

// class Frontend_impl : public Frontend_skel {
//     public:
//         Frontend_impl(std::shared_ptr<CORBA::ORB> orb);
//         CORBA::async<void> hello() override;
// };
