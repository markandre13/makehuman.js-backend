#pragma once

#include "../lib/threadsync.hh"

#include <opencv2/opencv.hpp>
#include <memory>
#include <functional>
#include <ev.h>
#include <corba/coroutine.hh>

class VideoCamera_impl;
class VideoReader;


class OpenCVLoop;

struct AsyncWatcher {
    ev_async watcher;
    OpenCVLoop *loop;
};

/**
 * Integrates OpenCV into the libev event loop
 * 
 * The OpenCV loop runs in the main thread (needed for macOS to be able to display windows)
 * while libev runs in another thread.
 */
class OpenCVLoop {
    struct ev_loop *_loop;
    ThreadSync _waitForOpenCVLoop;
    ThreadSync _waitForLibEVLoop;
    AsyncWatcher asyncWatcher;
    static void libev_async_cb(struct ev_loop *loop, struct ev_async *watcher, int revents);

    // used to suspend/resume the loop
    std::mutex _mutex;
    bool _running;
    std::shared_ptr<VideoCamera_impl> _next_camera;
    std::shared_ptr<VideoCamera_impl> _camera;

    std::shared_ptr<VideoReader> _next_reader;
    std::shared_ptr<VideoReader> _reader;

    cv::VideoCapture _capture;

    void waitForChange();
public:
    OpenCVLoop(struct ev_loop *loop);
    /**
     * init libev async event watcher (to be called from main thread / loop... do we really have to?)
     */
    void initAsync();

    /**
     * From the libev loop, execute cmd from within the OpenCV loop
     * 
     * * suspends function until next OpenCV loop run
     * * OpenCV loop resumes function
     * * suspends function until and schedules libev async event
     * * libev loop resumes function
     */
    template <typename T>
    CORBA::async<T> execute(std::function<T()> cmd) {
        // std::println("OpenCVLoop::execute(): SUSPEND OPENCV");
        // wait for this function to be executed from within the OpenCV thread
        co_await _waitForOpenCVLoop.suspend();
        // std::println("OpenCVLoop::execute(): RESUME OPENCV");
        T result = cmd();
        // wait for this function to be executed from within the libev thread
        // std::println("OpenCVLoop::execute(): SUSPEND LIBEV");
        co_await _waitForLibEVLoop.suspend();
        // std::println("OpenCVLoop::execute(): RESUME LIBEV");
        co_return result;
    }

    /**
     * From within the libev loop, execute cmd
     */
    CORBA::async<> executeLibEV(std::function<void()> cmd) {
        std::println("OpenCVLoop::executeLibEV(): SUSPEND LIBEV");
        co_await _waitForLibEVLoop.suspend();
        std::println("OpenCVLoop::executeLibEV(): RESUME LIBEV");
        cmd();
        std::println("OpenCVLoop::execute(): DONE");
    }

    // set by backend_impl
    std::function<void(const cv::Mat &frameImage, int32_t frameNumber)> frameHandler;
    /**
     * the event loop
     */
    void run();
    // video file to read from
    void setVideoReader(std::shared_ptr<VideoReader>);
    // camera to read from
    void setCamera(std::shared_ptr<VideoCamera_impl>);
    /**
     * unlock the mutex to resume the loop
     */
    void resume();
    /**
     * return from run()
     */
    inline void stop() { _running = false; }
};
