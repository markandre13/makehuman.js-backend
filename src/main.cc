#ifdef HAVE_MEDIAPIPE

#include <cc_lib/mediapipe.hh>
using mediapipe::cc_lib::vision::core::RunningMode;
using mediapipe::cc_lib::vision::face_landmarker::FaceLandmarker;
using mediapipe::cc_lib::vision::face_landmarker::FaceLandmarkerOptions;
using mediapipe::cc_lib::vision::face_landmarker::FaceLandmarkerResult;

#endif

#include <thread>
#include <sys/time.h>

#include <corba/corba.hh>
#include <corba/net/ws.hh>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <print>
#include <set>
#include <span>

#include "chordata/chordata.hh"
#include "livelink/livelink.hh"
#include "livelink/livelinkframe.hh"
#include "makehuman_impl.hh"

#ifdef HAVE_METAL
#include "metal/metal.hh"
#endif

using namespace std;

static uint64_t getMilliseconds();

int main(void) {
    println("makehuman.js backend");

    //
    // SETUP ORB
    //
    auto orb = make_shared<CORBA::ORB>();
    orb->debug = true;

    struct ev_loop *loop = EV_DEFAULT;
    println("the audience is listening...");

    auto backend = make_shared<Backend_impl>(orb);
    orb->bind("Backend", backend);

    auto protocol = new CORBA::net::WsProtocol();
    protocol->listen(orb.get(), loop, "localhost", 9001);

    auto chordata = new Chordata(loop, 6565, [&](const char *buffer, size_t nbytes) {
        // println("got chordata packet of {} bytes", nbytes);
        backend->chordata(buffer, nbytes);
    });

#ifdef HAVE_METAL
    MetalFacerenderer *metalRenderer = metal();
#ifndef HAVE_MEDIAPIPE
    auto facecap = new LiveLink(loop, 11111, [&](const LiveLinkFrame &frame) {
        // println("frame {}.{}", frame.frame, frame.subframe);
        metalRenderer->faceLandmarks(frame);
    });
#endif
#endif

std::thread libevthread(ev_run, loop, 0);

#ifdef HAVE_MEDIAPIPE
    //
    // SETUP MEDIAPIPE
    //

    auto options = std::make_unique<FaceLandmarkerOptions>();
    options->base_options.model_asset_path = "/Users/mark/python/py311-venv-mediapipe/face_landmarker_v2_with_blendshapes.task";
    options->running_mode = RunningMode::LIVE_STREAM;
    options->output_face_blendshapes = true;
    options->output_facial_transformation_matrixes = true;
    options->num_faces = 1;

    options->result_callback = [&](auto result, auto timestamp_ms) {

    // TODO: UPDATE METAL
#ifdef HAVE_METAL
        metalRenderer->faceLandmarks(result, timestamp_ms);
#endif
        // rendering has too much latency (reason: the table in the expression tab)
        // latency (cummulative) the frame was captured (3.5 GHz Intel Xeon E5)
        // * mediapipe face landmarker: 5ms when no face detected, 17ms with face (12 threads)
        // * received by browser      : 17ms to 100ms
        // * render latency           : 62ms to 112ms
        // (render latency does not include skipped frames)
        // println("latency: {}ms, thread count: {}", getMilliseconds() - timestamp_ms, tids.size());
        backend->faceLandmarks(result, timestamp_ms);
    };
    auto landmarker = FaceLandmarker::Create(std::move(options));
#endif

    //
    // SETUP VIDEO CAMERA
    //

    cv::VideoCapture cap;
    int deviceID = 0;         // 0 = open default camera
    int apiID = cv::CAP_ANY;  // 0 = autodetect default API
    cap.open(deviceID, apiID);
    if (!cap.isOpened()) {
        cerr << "failed to open video" << endl;
        return 1;
    }

    double w = cap.get(cv::CAP_PROP_FRAME_WIDTH) / 2;
    double h = cap.get(cv::CAP_PROP_FRAME_HEIGHT) / 2;
    auto backendName = cap.getBackendName();
    cap.set(cv::CAP_PROP_FRAME_WIDTH, w);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, h);
    cap.set(cv::CAP_PROP_FPS, 60);

    double fps = cap.get(cv::CAP_PROP_FPS);
    println("{}: {}x{}, {} fps", backendName.c_str(), w, h, fps);

    //
    // STREAM MEDIAPIPE'S FACE LANDMARKS TO FRONTEND
    //

    // namedWindow() doesn't work, but cv::waitKey() will display the window
    // cv::namedWindow("image");

    // https://developer.apple.com/documentation/corefoundation/cffiledescriptor?language=objc

    cv::Mat frame;
    while (true) {
        cap >> frame;
        if (frame.empty()) {
            std::cout << "empty image" << std::endl;
            continue;
        }
        auto timestamp = getMilliseconds();

        cv::imshow("image", frame);
#ifdef HAVE_MEDIAPIPE
        landmarker->DetectAsync(frame.channels(), frame.cols, frame.rows, frame.step, frame.data, timestamp);
#else
        // ev_run(loop, EVRUN_NOWAIT);
#endif
        cv::waitKey(1);  // wait 1ms (this also runs the cocoa eventloop)
    }

    return 0;
}

uint64_t getMilliseconds() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)(tv.tv_sec) * 1000 + (uint64_t)(tv.tv_usec) / 1000;
}