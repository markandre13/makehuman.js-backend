#include <opencv2/opencv.hpp>

#include <cc_lib/mediapipe.hh>
using mediapipe::cc_lib::vision::core::RunningMode;
using mediapipe::cc_lib::vision::face_landmarker::FaceLandmarkerOptions;
using mediapipe::cc_lib::vision::face_landmarker::FaceLandmarker;
using mediapipe::cc_lib::vision::face_landmarker::FaceLandmarkerResult;

#include <corba/corba.hh>

#include <corba/net/ws.hh>

#include <iostream>
#include <print>
#include <span>
#include <sys/time.h>

#include "makehuman_impl.hh"

using namespace std;

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
        ev_run(loop, EVRUN_NOWAIT);
        backend->faceLandmarks(result, timestamp_ms);
    };
    auto landmarker = FaceLandmarker::Create(std::move(options));

    //
    // SETUP VIDEO CAMERA
    //

    cv::VideoCapture cap;
    int deviceID = 0;             // 0 = open default camera
    int apiID = cv::CAP_ANY;      // 0 = autodetect default API
    cap.open(deviceID, apiID);
    if (!cap.isOpened()) {
        cerr << "failed to open video" << endl;
        return 1;
    }

    double w = cap.get(cv::CAP_PROP_FRAME_WIDTH) / 2;
    double h = cap.get(cv::CAP_PROP_FRAME_HEIGHT) / 2;
    double fps = cap.get(cv::CAP_PROP_FPS);
    auto backendName = cap.getBackendName();
    cap.set(cv::CAP_PROP_FRAME_WIDTH, w);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, h);

    println("{}: {}x{}, {} fps", backendName.c_str(), w, h, fps);

    //
    // STREAM MEDIAPIPE'S FACE LANDMARKS TO FRONTEND
    //

    // namedWindow() doesn't work, but cv::waitKey() will display the window
    // cv::namedWindow("image");

    cv::Mat frame;
    while (true) {
        cap >> frame;
        if (frame.empty()) {
            std::cout << "empty image" << std::endl;
            return 1;
        }

        struct timeval tv;
        gettimeofday(&tv, NULL);
        uint64_t timestamp = (uint64_t)(tv.tv_sec) * 1000 + (uint64_t)(tv.tv_usec) / 1000;

        cv::imshow("image", frame);
        landmarker->DetectAsync(frame.channels(), frame.cols, frame.rows, frame.step, frame.data, timestamp);

        cv::waitKey(1); // wait 1ms
    }

    return 0;
}
