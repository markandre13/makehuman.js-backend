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
        bool _paused = true;

    public:
        VideoReader(const std::string_view &filename);
        uint16_t fps() const;
        uint32_t frameCount() const;
        void seek(uint32_t frame);
        uint32_t tell();
        void reset();
        VideoReader &operator>>(cv::Mat &image);
        /**
         * time to wait before displaying the next frame
         * 
         * 0: indefinitly
         */
        int delay() const;
};
