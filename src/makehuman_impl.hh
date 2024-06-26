#pragma once

#include "makehuman_skel.hh"
#include <ev.h>

#ifdef HAVE_MEDIAPIPE
#include <cc_lib/mediapipe.hh>
#endif

class CaptureEngine;
class LiveLinkFrame;

class Backend_impl : public Backend_skel {
        struct ev_loop *loop;
        // std::atomic<std::shared_ptr<Frontend>> frontend;
        std::shared_ptr<Frontend> frontend;
        std::unique_ptr<CaptureEngine> body;
        std::unique_ptr<CaptureEngine> face;
        std::unique_ptr<CaptureEngine> hand;

        bool blendshapeNamesHaveBeenSend = false;
    public:
        Backend_impl(std::shared_ptr<CORBA::ORB> orb, struct ev_loop *loop);
        CORBA::async<> setFrontend(std::shared_ptr<Frontend> frontend) override;
        CORBA::async<> setEngine(MotionCaptureType type, MotionCaptureEngine engine) override;

        void chordata(const char *buffer, size_t nbytes);
        void livelink(LiveLinkFrame &frame);

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
