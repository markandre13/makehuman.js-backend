#pragma once

#include <ev.h>
#include <mutex>
#include "opencv/videowriter.hh"
#include "opencv/videoreader.hh"

#include "makehuman_skel.hh"
#include "mediapipe/blazepose.hh"

#ifdef HAVE_MEDIAPIPE
#include <cc_lib/mediapipe.hh>
#endif

class CaptureEngine;
class LiveLinkFrame;
class MoCapPlayer;
class VideoCamera_impl;

class OpenCVLoop {
    std::mutex _mutex;
    bool _running;
    std::shared_ptr<VideoCamera_impl> _next_camera;
    std::shared_ptr<VideoCamera_impl> _camera;
    cv::VideoCapture _capture;
public:
    void run();
    void setCamera(std::shared_ptr<VideoCamera_impl>);
    inline void stop() { _running = false; }
};

/**
 * big ball of mud representing the backend
 * 
 * todo: split up
 */
class Backend_impl : public Backend_skel {
        struct ev_loop *loop;
        OpenCVLoop *openCVLoop;
        // std::atomic<std::shared_ptr<Frontend>> frontend;
        std::shared_ptr<Frontend> frontend;

        std::unique_ptr<CaptureEngine> body;
        std::unique_ptr<CaptureEngine> face;
        std::unique_ptr<CaptureEngine> hand;

        bool blendshapeNamesHaveBeenSend = false;

        std::vector<std::shared_ptr<VideoCamera2>> cameras;

        std::shared_ptr<VideoWriter> videoWriter;
        std::shared_ptr<VideoReader> videoReader;
        std::shared_ptr<MoCapPlayer> mocapPlayer;
        void _stop();

    public:
        Backend_impl(std::shared_ptr<CORBA::ORB>, struct ev_loop *loop, OpenCVLoop *openCVLoop);
        CORBA::async<> setFrontend(std::shared_ptr<Frontend> frontend) override;
        CORBA::async<> setEngine(MotionCaptureType type, MotionCaptureEngine engine) override;
        CORBA::async<> save(const std::string_view &filename, const std::string_view &data) override;
        CORBA::async<std::string> load(const std::string_view &filename) override;

        CORBA::async<std::vector<std::shared_ptr<VideoCamera2>>> getVideoCameras() override;
        CORBA::async<> setCamera(std::shared_ptr<VideoCamera2> camera) override;
        
        CORBA::async<void> record(const std::string_view & filename) override;
        CORBA::async<Range> play(const std::string_view & filename) override;
        CORBA::async<void> stop() override;
        CORBA::async<void> pause() override;
        CORBA::async<void> seek(uint64_t timestamp_ms) override;

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
        void poseLandmarks(const BlazePose &pose, int64_t timestamp_ms);
};

// this would only be needed for testing

// class Frontend_impl : public Frontend_skel {
//     public:
//         Frontend_impl(std::shared_ptr<CORBA::ORB> orb);
//         CORBA::async<void> hello() override;
// };
