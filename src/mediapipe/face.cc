#include "face.hh"

using mediapipe::cc_lib::vision::core::RunningMode;
using mediapipe::cc_lib::vision::face_landmarker::FaceLandmarker;
using mediapipe::cc_lib::vision::face_landmarker::FaceLandmarkerOptions;
using mediapipe::cc_lib::vision::face_landmarker::FaceLandmarkerResult;

MediapipeFace::MediapipeFace(std::function<void(std::optional<FaceLandmarkerResult>, int64_t timestamp_ms)> callback) {
    auto options = std::make_unique<FaceLandmarkerOptions>();
    options->base_options.model_asset_path = "upstream/mediapipe_cc_lib/cc_lib/face_landmarker.task";
    options->running_mode = RunningMode::LIVE_STREAM;
    options->output_face_blendshapes = true;
    options->output_facial_transformation_matrixes = true;
    options->num_faces = 1;
    options->result_callback = callback;
    mp = FaceLandmarker::Create(std::move(options));
}
MediapipeFace::~MediapipeFace() {}
