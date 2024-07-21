#include "pose.hh"

#include <iostream>

using namespace std;

using mediapipe::cc_lib::vision::core::RunningMode;
using mediapipe::cc_lib::vision::pose_landmarker::PoseLandmarker;
using mediapipe::cc_lib::vision::pose_landmarker::PoseLandmarkerOptions;
using mediapipe::cc_lib::vision::pose_landmarker::PoseLandmarkerResult;

MediapipePose::MediapipePose(std::function<void(std::optional<PoseLandmarkerResult>, int64_t timestamp_ms)> callback) {
    auto options = std::make_unique<PoseLandmarkerOptions>();
    options->base_options.model_asset_path = "upstream/mediapipe_cc_lib/cc_lib/pose_landmarker_full.task";
    options->running_mode = RunningMode::LIVE_STREAM;
    options->num_poses = 1;
    options->result_callback = callback;
    mp = PoseLandmarker::Create(std::move(options));
    if (!mp) {
        throw runtime_error("failed to create pose landmarker");
    }
}
MediapipePose::~MediapipePose() {}
