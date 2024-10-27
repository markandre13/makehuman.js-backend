#pragma once

#include "videosource.hh"

class VideoCamera : public VideoSource {
    private:
        cv::VideoCapture cap;
        int deviceID;
        int apiID;

    public:
        VideoCamera(int deviceID = 0, int apiID = cv::CAP_ANY) : deviceID(deviceID), apiID(apiID) { open(); }
        VideoSource &operator>>(cv::Mat &image) override;
        double fps() override;
        void reset() override;
        int delay() const override;

    private:
        void open();
};
