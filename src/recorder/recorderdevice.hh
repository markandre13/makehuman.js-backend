#pragma once

#include "../capturedevice.hh"

class RecorderDevice : public virtual HolisticDevice_impl {
    public:
        static RecorderDevice *instance;
        RecorderDevice(struct ev_loop* loop);
        RecorderDevice(struct ev_loop* loop, unsigned port);
        std::string id() override;
        CaptureDeviceType type() override;
        std::string name() override;

        void landmarks(const std::span<float> & blendshapes, const std::span<float> & pose, const std::span<float> & lhand, const std::span<float> & rhand, uint64_t timestamp_ms);
};