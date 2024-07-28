#pragma once

#include "videosource.hh"

class VideoCamera : public VideoSource {
        int deviceID;
        int apiID;

    public:
        VideoCamera(int deviceID = 0, int apiID = cv::CAP_ANY) : deviceID(deviceID), apiID(apiID) { open(); }
        void reset() override;
        int delay() const override;

    private:
        void open();
};
