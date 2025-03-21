#pragma once

#include <opencv2/opencv.hpp>
#include <string>
#include <cstdint>

class VideoReader {
    private:
        cv::VideoCapture cap;
        uint64_t startTime = 0;
        uint64_t frameNumber = 0;
        double step = 0;

    public:
        VideoReader(const std::string_view &filename);
        double fps() const;
        double frameCount() const;
        void reset();
        VideoReader &operator>>(cv::Mat &image);
        int delay() const;
};
