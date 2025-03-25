#include "loop.hh"
#include "videoreader.hh"
#include "../util.hh"
#include "../macos/video/videocamera_impl.hh"

void OpenCVLoop::setCamera(std::shared_ptr<VideoCamera_impl> camera) {
    atomic_store(&_next_camera, camera);
    _mutex.unlock();
}

void OpenCVLoop::setVideoReader(std::shared_ptr<VideoReader> reader) {
    atomic_store(&_next_reader, reader);
    _mutex.unlock();
}

// TODO: move the video camera code into the video camera class
void OpenCVLoop::run() {
    _running = true;

    const char *windowName = "image";
    cv::Mat frame;

    // uint64_t lastFrameRead;
    // uint64_t frameReaderStart;
    uint64_t frameNumber;

    bool haveWindow = false;

    while (_running) {
        auto next_reader = std::atomic_load(&_next_reader);
        if (_reader != next_reader) {
            _reader = next_reader;
            // frameReaderStart = getMilliseconds();
            frameNumber = 0;
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
                frameHandler(frame, getMilliseconds());
            }
            ++frameNumber;
            if (!haveWindow) {
                cv::namedWindow(windowName, cv::WINDOW_AUTOSIZE);
                haveWindow = true;
            }
            cv::imshow(windowName, frame);
            // ++frameNumber;
            // auto now = getMilliseconds();
            // auto delay = frameNumber * 1000.0 / _reader->fps() - now;
            // if (delay < 1) {
            //     delay = 1;
            // }
            // cv::waitKey(delay);
            cv::waitKey(_reader->delay());
            continue;
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
            cv::destroyWindow(windowName);
            haveWindow = false;
        }

        cv::waitKey(1);
        // TODO: chance to race condition
        if (std::atomic_load(&_next_reader) || std::atomic_load(&_next_camera)) {
            continue;
        }
        _mutex.lock();  // wait for a change
    }
}
