#pragma once

#include <stdexcept>
#include <string>
#include <opencv2/opencv.hpp>

class VideoSource {
    public:
        virtual double fps() = 0;
        virtual VideoSource &operator>>(cv::Mat &image) = 0;
        // to be called when frame is empty
        virtual void reset() = 0;
        // delay in milliseconds till next frame
        virtual int delay() const = 0;
};
