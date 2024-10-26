#pragma once

#include <cstdint>

class CaptureEngine {
    public:
        virtual ~CaptureEngine();
};

class VideoCaptureEngine: CaptureEngine {
    public:
        virtual void frame(int channels, int width, int height, int width_step, uint8_t *pixel_data, int64_t timestamp_ms) = 0;
};
