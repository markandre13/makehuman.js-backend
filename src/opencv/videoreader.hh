#pragma once

#include "videosource.hh"
#include <string>
#include <cstdint>

class VideoReader : public VideoSource {
        uint64_t startTime = 0;
        uint64_t frameNumber = 0;
        double step = 0;

    public:
        VideoReader(const std::string &filename);
        void reset() override;
        VideoSource &operator>>(cv::Mat &image) override;
        int delay() const override;
};
