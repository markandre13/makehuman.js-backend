#include <corba/net/ws/protocol.hh>
#include <corba/util/logger.hh>
#include <iostream>
#include <mutex>
#include <opencv2/opencv.hpp>
#include <print>
#include <set>
#include <span>
#include <thread>

#include "makehuman_impl.hh"
#include "fs_impl.hh"
#include "macos/video/videocamera_impl.hh"

#include "ev/timer.hh"
#include "chordata/chordata.hh"
#include "freemocap/freemocap.hh"
#include "livelink/livelink.hh"
#include "livelink/livelinkframe.hh"
#include "mediapipe/face.hh"
#include "mediapipe/pose.hh"
#include "opencv/videoreader.hh"
#include "util.hh"

#ifdef HAVE_METAL
#include "macos/metal/metal.hh"
#endif

using namespace std;

int main(void) {
    // Logger::setLevel(LOG_DEBUG);

    println("makehuman.js backend");

    //
    // SETUP ORB
    //
    auto orb = make_shared<CORBA::ORB>();
    // orb->debug = true;

    struct ev_loop *loop = EV_DEFAULT;
    println("the audience is listening...");

    auto protocol = new CORBA::detail::WsProtocol(loop);
    orb->registerProtocol(protocol);
    protocol->listen("localhost", 9001);

    OpenCVLoop openCVLoop;

    auto backend = make_shared<Backend_impl>(orb, loop, &openCVLoop);
    orb->bind("Backend", backend);

    auto filesystem = make_shared<FileSystem_impl>();
    orb->bind("FileSystem", filesystem);

#if 0
    auto chordata = new Chordata(loop, 6565, [&](const char *buffer, size_t nbytes) {
        // println("got chordata packet of {} bytes", nbytes);
        backend->chordata(buffer, nbytes);
    });

    // #ifdef HAVE_METAL
    //     MetalFacerenderer *metalRenderer = metal();
    // #ifndef HAVE_MEDIAPIPE
    //     auto facecap = new LiveLink(loop, 11111, [&](const LiveLinkFrame &frame) {
    //         // println("frame {}.{}", frame.frame, frame.subframe);
    //         metalRenderer->faceLandmarks(frame);
    //     });
    // #endif
    // #endif

    // auto landmarker = make_unique<MediapipeFace>([&](auto result, int64_t timestamp_ms) {
    //     backend->faceLandmarks(result, timestamp_ms);
    // });

    auto landmarker = make_unique<MediapipePose>([&](auto result, int64_t timestamp_ms) {
        // backend->poseLandmarks(result, timestamp_ms);
    });
#endif

#if 0
    auto filename =
        "/Users/mark/freemocap_data/recording_sessions/session_2024-10-06_13_24_28/recording_13_29_02_gmt+2__drei/"
        "output_data/mediapipe_body_3d_xyz.csv";
    FreeMoCap freemocap(filename);
    BlazePose pose;
    Timer timer(loop, 0.0, 1.0 / 15.0, [&] {
        println("timer");
        freemocap.getPose(&pose);
        auto timestamp_ms = getMilliseconds();
        backend->poseLandmarks(pose, timestamp_ms);
    });
#endif

    println("enter libev thread");

    // auto pulseTimer = make_unique<Timer>(loop, 0, 1, [] {
    //     println("tick");
    // });

    std::thread libevthread(ev_run, loop, 0);
#if 1
    openCVLoop.run();
    libevthread.join();
    println("left libev thread");
#else
    // capture in a thread, render in main?
    // VideoReader cap("video.mp4");
    int deviceID = 0;
    VideoCamera cap(deviceID);
    double fps = cap.fps();

    cv::Mat frame;
    unsigned frameCounter = 0;

    while (true) {
        bool frameFromFile = false;
        if (!backend->readFrame(frame)) {
            // println("get frame from camera");
            cap >> frame;
            if (frame.empty()) {
                cap.reset();
                continue;
            }
        } else {
            // ++frameCounter;
            // println("got frame from file");
            if (frameCounter == 30 * 5 + 12 || frame.empty()) {
                backend->reset();
                frameCounter = 0;
                continue;
            }
            frameFromFile = true;
        }

        auto timestamp = getMilliseconds();

        backend->saveFrame(frame, fps);

        cv::imshow("image", frame);
        // landmarker->frame(frame.channels(), frame.cols, frame.rows, frame.step, frame.data, timestamp);

        auto now = getMilliseconds();
        auto delay = 1000.0 / fps - (now - timestamp);
        if (delay < 1) {
            delay = 1;
        }

        cv::waitKey(frameFromFile ? backend->delay() : cap.delay());  // wait 1ms (this also runs the cocoa eventloop)
    }
#endif
    return 0;
}

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