#pragma once

#include <ev.h>
#include "opencv/videowriter.hh"
#include "opencv/videoreader.hh"

#include "makehuman_skel.hh"

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

        std::shared_ptr<VideoWriter> videoWriter;
        std::shared_ptr<VideoReader> videoReader;
        void _stop();

    public:
        Backend_impl(std::shared_ptr<CORBA::ORB> orb, struct ev_loop *loop);
        CORBA::async<> setFrontend(std::shared_ptr<Frontend> frontend) override;
        CORBA::async<> setEngine(MotionCaptureType type, MotionCaptureEngine engine) override;
        CORBA::async<> save(const std::string_view &filename, const std::string_view &data) override;
        CORBA::async<std::string> load(const std::string_view &filename) override;
        
        CORBA::async<void> record(const std::string_view & filename) override;
        CORBA::async<void> play(const std::string_view & filename) override;
        CORBA::async<void> stop() override;
        bool readFrame(cv::Mat &frame);
        int delay();
        void reset();
        void saveFrame(const cv::Mat &frame, double fps);

        void chordata(const char *buffer, size_t nbytes);
        void livelink(LiveLinkFrame &frame);

#ifdef HAVE_MEDIAPIPE
        void faceLandmarks(std::optional<mediapipe::cc_lib::vision::face_landmarker::FaceLandmarkerResult> result, int64_t timestamp_ms);
        void poseLandmarks(std::optional<mediapipe::cc_lib::vision::pose_landmarker::PoseLandmarkerResult> result, int64_t timestamp_ms);
#endif
};

// this would only be needed for testing

// class Frontend_impl : public Frontend_skel {
//     public:
//         Frontend_impl(std::shared_ptr<CORBA::ORB> orb);
//         CORBA::async<void> hello() override;
// };
