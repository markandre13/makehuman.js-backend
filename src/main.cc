#include <corba/net/ws/protocol.hh>
#include <corba/util/logger.hh>
#include <iostream>
#include <mutex>
#include <opencv2/opencv.hpp>
#include <print>
#include <set>
#include <span>
#include <thread>

#include "backend_impl.hh"
#include "chordata/chordata.hh"
#include "ev/timer.hh"
#include "freemocap/freemocap.hh"
#include "fs_impl.hh"
#include "livelink/livelink.hh"
#include "livelink/livelinkframe.hh"
#include "macos/video/videocamera_impl.hh"

#if HAVE_MEDIAPIPE
#include "mediapipe/face.hh"
#include "mediapipe/pose.hh"
#endif

#include "opencv/loop.hh"
#include "opencv/videoreader.hh"
#include "util.hh"

#ifdef HAVE_METAL
#include "macos/metal/metal.hh"
#endif

using namespace std;

void XinitAsync();

void run(OpenCVLoop* openCvLoop, struct ev_loop* libEvLoop) {
    openCvLoop->initAsync();
    XinitAsync();
    ev_run(libEvLoop, 0);
}

int main(void) {
    // Logger::setLevel(LOG_DEBUG);

    println("makehuman.js backend");

    //
    // SETUP ORB
    //
    auto orb = make_shared<CORBA::ORB>();
    // orb->debug = true;

    struct ev_loop* loop = EV_DEFAULT;
    println("the audience is listening...");

    auto protocol = new CORBA::detail::WsProtocol(loop);
    orb->registerProtocol(protocol);
    protocol->listen("localhost", 9001);

    OpenCVLoop openCVLoop(loop);

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

    // auto pulseTimer = make_unique<Timer>(loop, 0, 1, [] {
    //     println("tick");
    // });

    std::thread libevthread(run, &openCVLoop, loop);
    openCVLoop.run();
    libevthread.join();

    return 0;
}
