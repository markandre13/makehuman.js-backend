#pragma once

#include <opencv2/opencv.hpp>
#include <memory>
#include <functional>
#include <mutex>

class VideoCamera_impl;
class VideoReader;

/**
 * OpenCV event loop
 * 
 * Needs to run in the main thread.
 */
class OpenCVLoop {
    std::mutex _mutex;
    bool _running;
    std::shared_ptr<VideoCamera_impl> _next_camera;
    std::shared_ptr<VideoCamera_impl> _camera;

    std::shared_ptr<VideoReader> _next_reader;
    std::shared_ptr<VideoReader> _reader;

    cv::VideoCapture _capture;

    void waitForChange();
public:
    std::function<void(const cv::Mat &frameImage, int32_t frameNumber)> frameHandler;
    void run();
    void setVideoReader(std::shared_ptr<VideoReader>);
    void setCamera(std::shared_ptr<VideoCamera_impl>);
    void resume();
    inline void stop() { _running = false; }
};
