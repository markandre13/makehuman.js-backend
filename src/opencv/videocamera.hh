#pragma once

#include "../generated/makehuman_skel.hh"
#include "videosource.hh"



/**
 * \deprecated By VideoCamera(2) in IDL
 */
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
