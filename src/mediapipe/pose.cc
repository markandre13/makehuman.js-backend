#include "pose.hh"
#include "../makehuman_impl.hh"
#include <iostream>

using namespace std;

using mediapipe::cc_lib::vision::core::RunningMode;
using mediapipe::cc_lib::vision::pose_landmarker::PoseLandmarker;
using mediapipe::cc_lib::vision::pose_landmarker::PoseLandmarkerOptions;
using mediapipe::cc_lib::vision::pose_landmarker::PoseLandmarkerResult;

MediapipePose::MediapipePose(Backend_impl *backend): MediaPipeTask_impl(backend) {
    auto options = std::make_unique<PoseLandmarkerOptions>();
    options->base_options.model_asset_path = "upstream/mediapipe_cc_lib/cc_lib/pose_landmarker_full.task";
    options->running_mode = RunningMode::LIVE_STREAM;
    options->num_poses = 1;
    options->result_callback = [&](std::optional<PoseLandmarkerResult> result, int64_t timestamp_ms) {
        if (!result.has_value() || result->pose_landmarks.size() == 0) {
            return;
        }
        auto frontend = _backend->getFrontend();
        if (!frontend) {
            return;
        }

        auto &lm = result->pose_world_landmarks[0].landmarks;
        float lm_array[lm.size() * 3];
        float *ptr = lm_array;
        for (size_t i = 0; i < lm.size(); ++i) {
            *(ptr++) = lm[i].x;
            *(ptr++) = lm[i].y;
            *(ptr++) = lm[i].z;
        }
        std::span landmarks{lm_array, lm.size() * 3zu};
        frontend->poseLandmarks(landmarks, timestamp_ms);
    };
    mp = PoseLandmarker::Create(std::move(options));
    if (!mp) {
        throw runtime_error("failed to create pose landmarker");
    }
}

CORBA::async<std::string> MediapipePose::name() { co_return "pose"; }

void MediapipePose::frame(const cv::Mat &frame, int64_t timestamp_ms) {
    mp->DetectAsync(frame.channels(), frame.cols, frame.rows, frame.step, frame.data, timestamp_ms);
}
