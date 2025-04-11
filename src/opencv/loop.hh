#pragma once

#include <opencv2/opencv.hpp>
#include <memory>
#include <functional>
#include <mutex>
#include <queue>
#include <ev.h>
#include <corba/coroutine.hh>

class VideoCamera_impl;
class VideoReader;

class ThreadSync {
    std::vector<std::coroutine_handle<>> _queue;
    std::mutex _mutex;
    std::function<void()> _schedule;
  
    struct awaitable {
        ThreadSync *_owner;
        awaitable(ThreadSync *owner) : _owner(owner) {}
        bool await_ready() { return false; }
        void await_suspend(std::coroutine_handle<> h) { _owner->add(h); }
        void await_resume() { }
    };
    void add(std::coroutine_handle<> handle) {
        _mutex.lock();
        _queue.push_back(handle);
        _schedule();
        _mutex.unlock();
    }
  
  public:
    ThreadSync(std::function<void()> schedule): _schedule(schedule) {}
    awaitable suspend() { return awaitable{this}; }
    void resume() {
        std::vector<std::coroutine_handle<>> queue;
        _mutex.lock();
        queue.swap(_queue);
        _mutex.unlock();
        for (auto &handle : queue) {
            handle.resume();
        }
    }
};

class OpenCVLoop;

struct AsyncWatcher {
    ev_async watcher;
    OpenCVLoop *loop;
};

/**
 * OpenCV event loop
 * 
 * The OpenCV loop runs in the main thread (needed for macOS to be able to display windows)
 * while libev runs in another thread.
 */
class OpenCVLoop {
    ThreadSync _syncOpenCV;
    ThreadSync _syncLibEV;
    AsyncWatcher asyncWatcher;
    struct ev_loop *_loop;
    static void libev_async_cb(struct ev_loop *loop, struct ev_async *watcher, int revents);

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
    void initAsync();

    /**
     * Execute OpenCV code from the libev loop.
     */
    template <typename T>
    CORBA::async<T> execute(std::function<T()> cmd) {
        // std::println("OpenCVLoop::execute(): SUSPEND OPENCV");
        // wait for this function to be executed from within the OpenCV thread
        co_await _syncOpenCV.suspend();
        // std::println("OpenCVLoop::execute(): RESUME OPENCV");
        T result = cmd();
        // wait for this function to be executed from within the libev thread
        // std::println("OpenCVLoop::execute(): SUSPEND LIBEV");
        co_await _syncLibEV.suspend();
        // std::println("OpenCVLoop::execute(): RESUME LIBEV");
        co_return result;
    }

    CORBA::async<> executeLibEV(std::function<void()> cmd) {
        std::println("OpenCVLoop::executeLibEV(): SUSPEND LIBEV");
        co_await _syncLibEV.suspend();
        std::println("OpenCVLoop::executeLibEV(): RESUME LIBEV");
        cmd();
        std::println("OpenCVLoop::execute(): DONE");
    }

    std::function<void(const cv::Mat &frameImage, int32_t frameNumber)> frameHandler;
    void run();
    void setVideoReader(std::shared_ptr<VideoReader>);
    void setCamera(std::shared_ptr<VideoCamera_impl>);
    void resume();
    inline void stop() { _running = false; }
};
