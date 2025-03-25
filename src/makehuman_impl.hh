#pragma once

#include "generated/makehuman_skel.hh"

#include <ev.h>
#include <mutex>
#include "opencv/videowriter.hh"
#include "opencv/videoreader.hh"

#include "mediapipe/blazepose.hh"
#include "mediapipe/mediapipetask_impl.hh"

#ifdef HAVE_MEDIAPIPE
#include <cc_lib/mediapipe.hh>
#endif

class CaptureEngine;
class LiveLinkFrame;
class MoCapPlayer;
class Recorder_impl;
class VideoCamera_impl;

class OpenCVLoop {
    std::mutex _mutex;
    bool _running;
    std::shared_ptr<VideoCamera_impl> _next_camera;
    std::shared_ptr<VideoCamera_impl> _camera;

    std::shared_ptr<VideoReader> _next_reader;
    std::shared_ptr<VideoReader> _reader;

    cv::VideoCapture _capture;
public:
    std::function<void(const cv::Mat &frame, int64_t timestamp_ms)> frameHandler;
    void run();
    void setVideoReader(std::shared_ptr<VideoReader>);
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

        std::shared_ptr<Recorder_impl> _recorder;

        /**
         * List of available cameras
         */
        std::vector<std::shared_ptr<VideoCamera>> cameras;
        /**
         * Camera selected by the user
         */
        std::shared_ptr<VideoCamera_impl> _camera;

        /**
         * Set when recording to write video stream to file
         */
        std::shared_ptr<VideoWriter> _videoWriter;

        std::vector<std::shared_ptr<MediaPipeTask>> mediaPipeTasks;
        std::shared_ptr<MediaPipeTask_impl> _mediaPipeTask;

        std::unique_ptr<CaptureEngine> body;
        std::unique_ptr<CaptureEngine> face;
        std::unique_ptr<CaptureEngine> hand;

        bool blendshapeNamesHaveBeenSend = false;

        std::shared_ptr<VideoReader> videoReader;
        std::shared_ptr<MoCapPlayer> mocapPlayer;
        void _stop();

    public:
        Backend_impl(std::shared_ptr<CORBA::ORB>, struct ev_loop *loop, OpenCVLoop *openCVLoop);

        CORBA::async<> setFrontend(std::shared_ptr<Frontend> frontend) override;
        inline std::shared_ptr<Frontend> getFrontend() {
            return std::atomic_load(&frontend);
        }
        CORBA::async<std::shared_ptr<Recorder>> recorder() override;

        // CORBA::async<> setEngine(MotionCaptureType type, MotionCaptureEngine engine) override;
        // CORBA::async<> save(const std::string_view &filename, const std::string_view &data) override;
        // CORBA::async<std::string> load(const std::string_view &filename) override;

        CORBA::async<std::vector<std::shared_ptr<VideoCamera>>> getVideoCameras() override;
        CORBA::async<std::shared_ptr<VideoCamera>> camera() override;
        CORBA::async<> camera(std::shared_ptr<VideoCamera>) override;

        CORBA::async<std::vector<std::shared_ptr<MediaPipeTask>>> getMediaPipeTasks() override;
        CORBA::async<std::shared_ptr<MediaPipeTask>> mediaPipeTask() override;
        CORBA::async<void> mediaPipeTask(std::shared_ptr<MediaPipeTask>) override;

        bool readFrame(cv::Mat &frame);
        int delay();
        void reset();
        void saveFrame(const cv::Mat &frame, double fps);

        void chordata(const char *buffer, size_t nbytes);
        void livelink(LiveLinkFrame &frame);

        void poseLandmarks(const BlazePose &pose, int64_t timestamp_ms);
};
