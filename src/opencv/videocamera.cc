#include "videocamera.hh"
#include <print>

using namespace std;

void VideoCamera::reset() {
    cap.release();
    open();
}
int VideoCamera::delay() const {
    // wait as little as possible
    return 1;
}

void VideoCamera::open() {
    cap.open(deviceID, apiID);
    if (!cap.isOpened()) {
        throw runtime_error(format("failed to open video device {}, api {}", deviceID, apiID));
    }
    double w = cap.get(cv::CAP_PROP_FRAME_WIDTH) / 2;
    double h = cap.get(cv::CAP_PROP_FRAME_HEIGHT) / 2;
    auto backendName = cap.getBackendName();
    cap.set(cv::CAP_PROP_FRAME_WIDTH, w);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, h);
    cap.set(cv::CAP_PROP_FPS, 60);

    double fps = cap.get(cv::CAP_PROP_FPS);
    println("opened video capture device {}: {}x{}, {} fps", backendName.c_str(), w, h, fps);
}
