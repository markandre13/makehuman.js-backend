#pragma once

#include "../captureengine.hh"
#include <cc_lib/mediapipe.hh>

class MediapipePose : VideoCaptureEngine {
        std::unique_ptr<mediapipe::cc_lib::vision::pose_landmarker::PoseLandmarker> mp;
    public:
        MediapipePose(std::function<void(std::optional<mediapipe::cc_lib::vision::pose_landmarker::PoseLandmarkerResult>, int64_t timestamp_ms)>);
        virtual ~MediapipePose();

        void frame(int channels, int width, int height, int width_step, uint8_t *pixel_data, int64_t timestamp_ms) override {
            mp->DetectAsync(channels, width, height, width_step, pixel_data, timestamp_ms);
        }
};
