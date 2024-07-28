#include <sys/time.h>

#include <corba/corba.hh>
#include <corba/net/ws.hh>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <print>
#include <set>
#include <span>
#include <thread>

#include "chordata/chordata.hh"
#include "livelink/livelink.hh"
#include "livelink/livelinkframe.hh"
#include "makehuman_impl.hh"
#include "mediapipe/face.hh"
#include "mediapipe/pose.hh"

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
    // orb->debug = true;

    struct ev_loop *loop = EV_DEFAULT;
    println("the audience is listening...");

    auto protocol = new CORBA::net::WsProtocol();
    protocol->listen(orb.get(), loop, "localhost", 9001);

    auto backend = make_shared<Backend_impl>(orb, loop);
    orb->bind("Backend", backend);

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

    std::thread libevthread(ev_run, loop, 0);

    // auto landmarker = make_unique<MediapipeFace>([&](auto result, int64_t timestamp_ms) {
    //     backend->faceLandmarks(result, timestamp_ms);
    // });
    auto landmarker = make_unique<MediapipePose>([&](auto result, int64_t timestamp_ms) {
        backend->poseLandmarks(result, timestamp_ms);
    });

    //
    // SETUP VIDEO CAMERA
    //

    // macOS native APIs
    // Core Video / AVFoundation
    // getVideoInputs();

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
    println("opened video capture device {}: {}x{}, {} fps", backendName.c_str(), w, h, fps);

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
            cout << "empty image" << std::endl;
            continue;
        }

        auto timestamp = getMilliseconds();

        backend->frame(frame, fps);

        cv::imshow("image", frame);
        // landmarker->frame(frame.channels(), frame.cols, frame.rows, frame.step, frame.data, timestamp);
        cv::waitKey(1);  // wait 1ms (this also runs the cocoa eventloop)
    }

    return 0;
}

uint64_t getMilliseconds() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)(tv.tv_sec) * 1000 + (uint64_t)(tv.tv_usec) / 1000;
}