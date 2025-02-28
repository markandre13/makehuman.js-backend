#include <corba/corba.hh>
#include <corba/net/ws/protocol.hh>
#include <corba/util/logger.hh>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <print>
#include <set>
#include <span>
#include <thread>
#include <mutex>

#include "chordata/chordata.hh"
#include "ev/timer.hh"
#include "freemocap/freemocap.hh"
#include "livelink/livelink.hh"
#include "livelink/livelinkframe.hh"
#include "makehuman_impl.hh"
#include "mediapipe/face.hh"
#include "mediapipe/pose.hh"
#include "opencv/videocamera.hh"
#include "opencv/videoreader.hh"
#include "util.hh"
#include "macos/video/video.hh"

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

void OpenCVLoop::run() {
    _running = true;

    const char *windowName = "image";
    cv::Mat frame;

    while(_running) {
        auto next_camera = std::atomic_load(&_next_camera);
        if (_camera != next_camera) {
            if (_camera) {
                _capture.release();
            }
            if (!_camera && next_camera) {
                cv::namedWindow(windowName, cv::WINDOW_AUTOSIZE);
            }
            if (_camera && !next_camera) {
                cv::destroyWindow(windowName);
                
            }
            _camera = next_camera;
            if (_camera) {
                _capture.open(next_camera->openCvIndex());
            }
        }

        if (!_camera) {
            cv::waitKey(1);
            _mutex.lock();
            continue;
        }

        
        if (_capture.grab()) {
            auto timestamp_ms = getMilliseconds();
            if (_capture.retrieve(frame)) {
                cv::imshow(windowName, frame);
                if (frameHandler) {
                    frameHandler(frame, timestamp_ms);
                }
            }
        }

        // TODO: delay until next frame
        cv::waitKey(1);  // wait 1ms (this also runs the cocoa eventloop)
    }
}