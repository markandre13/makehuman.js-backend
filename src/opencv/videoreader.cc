#include "videoreader.hh"

#include "../util.hh"

using namespace std;

VideoReader::VideoReader(const string_view &filename) {
    cap.open(string(filename));
    if (!cap.isOpened()) {
        throw runtime_error(format("failed to open video file \"{}\"", filename));
    }
    double w = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    double h = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    double fps = cap.get(cv::CAP_PROP_FPS);
    double frameCount = cap.get(cv::CAP_PROP_FRAME_COUNT); //!< Number of frames in the video file.
    step = 1000.0 / fps;

    println("opened video file \"{}\": {}x{}, {} fps, {} frames", filename, w, h, fps, frameCount);
}

double VideoReader::fps() const {
    return cap.get(cv::CAP_PROP_FPS);
}

double VideoReader::frameCount() const {
    return cap.get(cv::CAP_PROP_FRAME_COUNT);
}

void VideoReader::reset() {
    cap.set(cv::CAP_PROP_POS_FRAMES, 0);
    startTime = 0;
}

VideoReader &VideoReader::operator>>(cv::Mat &image) {
    cap >> image;
    if (startTime == 0) {
        startTime = getMilliseconds();
        frameNumber = 1;
    } else {
        ++frameNumber;
    }
    return *this;
}

int VideoReader::delay() const {
    auto nextFrame = frameNumber * step;
    auto now = getMilliseconds();
    auto delay = nextFrame + startTime - now;

    // println("frameNumber {}: nextFrame {} + startTime {} - now {} = delay {}", frameNumber, nextFrame, startTime, now, delay);

    if (delay < 1) {
        delay = 1;
    }

    return delay;
}
