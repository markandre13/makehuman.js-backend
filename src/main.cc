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
#include "opencv/videocamera.hh"
#include "opencv/videoreader.hh"
#include "util.hh"

#ifdef HAVE_METAL
#include "metal/metal.hh"
#endif

using namespace std;

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

    // VideoReader cap("video.mp4");
    VideoCamera cap;
    double fps = cap.fps();

    cv::Mat frame;

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
            // println("got frame from file");
            if (frame.empty()) {
                backend->reset();
                continue;
            }
            frameFromFile = true;
        }

        auto timestamp = getMilliseconds();

        backend->saveFrame(frame, fps);

        cv::imshow("image", frame);
        landmarker->frame(frame.channels(), frame.cols, frame.rows, frame.step, frame.data, timestamp);

        auto now = getMilliseconds();
        auto delay = 1000.0 / fps - (now - timestamp);
        if (delay < 1) {
            delay = 1;
        }

        cv::waitKey(frameFromFile ? backend->delay() : cap.delay());  // wait 1ms (this also runs the cocoa eventloop)
    }

    return 0;
}
