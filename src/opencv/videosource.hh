#pragma once

#include <stdexcept>
#include <string>
#include <opencv2/opencv.hpp>

class VideoSource {
    protected:
        cv::VideoCapture cap;

    public:
        inline double fps() { return cap.get(cv::CAP_PROP_FPS); }
        virtual VideoSource &operator>>(cv::Mat &image);
        // to be called when frame is empty
        virtual void reset() = 0;
        // delay in milliseconds till next frame
        virtual int delay() const = 0;
};
