#include "loop.hh"
#include "videoreader.hh"
#include "../util.hh"
#include "../macos/video/videocamera_impl.hh"

#include <queue>

using namespace std;

OpenCVLoop::OpenCVLoop(struct ev_loop *loop): 
    _loop(loop),
    _syncOpenCV([this] {resume();}),
    _syncLibEV([this]{
        ev_async_send(_loop, &asyncWatcher.watcher);
        resume();
    })
{
    asyncWatcher.loop = this;
}

void OpenCVLoop::initAsync() {
    ev_async_init(&asyncWatcher.watcher, libev_async_cb);
    ev_async_start(_loop, &asyncWatcher.watcher);
}

void OpenCVLoop::libev_async_cb(struct ev_loop *loop, struct ev_async *watcher, int revents) {
    // println("OpenCVLoop::libev_async_cb()");
    auto w = reinterpret_cast<AsyncWatcher*>(watcher);
    w->loop->_syncLibEV.resume();
}

void OpenCVLoop::setCamera(std::shared_ptr<VideoCamera_impl> camera) {
    atomic_store(&_next_camera, camera);
    _mutex.unlock();
}

void OpenCVLoop::setVideoReader(std::shared_ptr<VideoReader> reader) {
    atomic_store(&_next_reader, reader);
    _mutex.unlock();
}

void OpenCVLoop::resume() {
    _mutex.unlock();
}

// TODO: move the video camera code into the video camera class
void OpenCVLoop::run() {
    _running = true;

    const char *windowName = "image";
    cv::Mat frame;

    bool haveWindow = false;

    while (_running) {

        // println("OpenCVLoop::run(): opencv resume");
        _syncOpenCV.resume();

        // println("OpenCVLoop::run(): loop");
        auto next_reader = std::atomic_load(&_next_reader);
        if (_reader != next_reader) {
            _reader = next_reader;
        }

        if (_reader) {
            *_reader >> frame;
            if (frame.empty()) {
                _reader->reset();
                // frameReaderStart = getMilliseconds();
                // frameNumber = 0;
                *_reader >> frame;
            }

            if (frameHandler) {
                // frameHandler(frame, frameNumber * 1000.0 / _reader->fps());
                auto frameNumber = _reader->tell();
                frameHandler(frame, frameNumber);
            }
            if (!haveWindow) {
                cv::namedWindow(windowName, cv::WINDOW_AUTOSIZE);
                haveWindow = true;
            }
            cv::imshow(windowName, frame);

            auto delay = _reader->delay();
            if (delay == 0) {
                waitForChange();
            } else {
                cv::waitKey(delay);
            }
            continue;
        // } else {
        //     println("OpenCVLoop::run(): no reader");
        }

        //
        // CAMERA
        //

        auto next_camera = std::atomic_load(&_next_camera);
        if (_camera != next_camera) {
            if (_camera) {
                _capture.release();
            }
            if (!haveWindow && !_camera && next_camera) {
                cv::namedWindow(windowName, cv::WINDOW_AUTOSIZE);
                haveWindow = true;
            }
            if (haveWindow && _camera && !next_camera) {
                cv::destroyWindow(windowName);
                haveWindow = false;
            }
            _camera = next_camera;
            if (_camera) {
                _capture.open(next_camera->openCvIndex());

                double w = _capture.get(cv::CAP_PROP_FRAME_WIDTH) / 2;
                double h = _capture.get(cv::CAP_PROP_FRAME_HEIGHT) / 2;
                _capture.set(cv::CAP_PROP_FRAME_WIDTH, w);
                _capture.set(cv::CAP_PROP_FRAME_HEIGHT, h);
                _capture.set(cv::CAP_PROP_FPS, 60);

                double fps = _capture.get(cv::CAP_PROP_FPS);
                _camera->fps(fps);
            }
        }

        if (_camera) {
            // on macos, grab() & retrieve() will block
            if (_capture.grab()) {
                auto timestamp_ms = getMilliseconds();
                if (_capture.retrieve(frame)) {
                    cv::imshow(windowName, frame);
                    if (frameHandler) {
                        frameHandler(frame, timestamp_ms);
                    }
                }
            }
            cv::waitKey(1);
            continue;
        }

        if (haveWindow) {
            // println("destroy window");
            cv::destroyWindow(windowName);
            haveWindow = false;
        }

        waitForChange();
    }
}

void OpenCVLoop::waitForChange() {
    // println("OpenCVLoop::waitForChange(): waitKey()");
    cv::waitKey(1);
    // TODO: chance to race condition
    if (std::atomic_load(&_next_reader) != _reader || std::atomic_load(&_next_camera) != _camera) {
        return;
    }
    // println("OpenCVLoop::waitForChange(): _mutex.lock()");
    _mutex.lock();  // wait for a change
    // println("OpenCVLoop::waitForChange(): _mutex unlocked");
}