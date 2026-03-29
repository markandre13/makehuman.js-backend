#include "recorderdevice.hh"

#include <print>
#include <string>

using namespace std;

RecorderDevice::RecorderDevice(struct ev_loop* loop) {
    println("RecorderDevice");
    instance = this;
}

RecorderDevice * RecorderDevice::instance = nullptr;

std::string RecorderDevice::id() { return "RecorderDevice"; }
CaptureDeviceType RecorderDevice::type() { return CaptureDeviceType::HOLISTIC; }
std::string RecorderDevice::name() { return "Recorder"; }

void RecorderDevice::landmarks(const std::span<float> & blendshapes, const std::span<float> & pose, const std::span<float> & lhand, const std::span<float> & rhand, uint64_t timestamp_ms) {
    if (_receiver) {
        _receiver->landmarks(blendshapes, pose, lhand, rhand, timestamp_ms);
    }
}
