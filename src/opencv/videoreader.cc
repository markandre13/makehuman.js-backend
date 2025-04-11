#include "videoreader.hh"

#include "../util.hh"

using namespace std;

VideoReader::VideoReader(const string filename) {
    println("VideoReader::VideoReader(\"{}\")", filename);
    cap.open(filename);
    if (!cap.isOpened()) {
        throw runtime_error(format("failed to open video file \"{}\"", filename));
    }
    // double w = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    // double h = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    double fps = cap.get(cv::CAP_PROP_FPS);
    step = 1000.0 / fps;
    // double frameCount = cap.get(cv::CAP_PROP_FRAME_COUNT); //!< Number of frames in the video file.
}

uint16_t VideoReader::fps() const {
    return static_cast<uint16_t>(cap.get(cv::CAP_PROP_FPS));
}

uint32_t VideoReader::frameCount() const {
    return static_cast<uint32_t>(cap.get(cv::CAP_PROP_FRAME_COUNT));
}

void VideoReader::seek(uint32_t frame) {
    cap.set(cv::CAP_PROP_POS_FRAMES, frame);
}

uint32_t VideoReader::tell() {
    return cap.get(cv::CAP_PROP_POS_FRAMES);
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
    // if (_paused) {
    //     return 0;
    // }

    auto nextFrame = frameNumber * step;
    auto now = getMilliseconds();
    auto delay = nextFrame + startTime - now;

    // println("frameNumber {}: nextFrame {} + startTime {} - now {} = delay {}", frameNumber, nextFrame, startTime, now, delay);

    if (delay < 1) {
        delay = 1;
    }

    return delay;
}
