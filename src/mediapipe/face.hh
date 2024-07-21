#pragma once

#include "../captureengine.hh"
#include <cc_lib/mediapipe.hh>

class MediapipeFace : CaptureEngine {
        std::unique_ptr<mediapipe::cc_lib::vision::face_landmarker::FaceLandmarker> mp;
    public:
        MediapipeFace(std::function<void(std::optional<mediapipe::cc_lib::vision::face_landmarker::FaceLandmarkerResult>, int64_t timestamp_ms)>);
        virtual ~MediapipeFace();

        inline void frame(int channels, int width, int height, int width_step, uint8_t *pixel_data, int64_t timestamp_ms) {
            mp->DetectAsync(channels, width, height, width_step, pixel_data, timestamp_ms);
        }
};
