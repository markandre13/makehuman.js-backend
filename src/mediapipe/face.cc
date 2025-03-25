#include "face.hh"
#include "../backend_impl.hh"

using mediapipe::cc_lib::vision::core::RunningMode;
using mediapipe::cc_lib::vision::face_landmarker::FaceLandmarker;
using mediapipe::cc_lib::vision::face_landmarker::FaceLandmarkerOptions;
using mediapipe::cc_lib::vision::face_landmarker::FaceLandmarkerResult;

MediapipeFace::MediapipeFace(Backend_impl *backend): MediaPipeTask_impl(backend) {
    auto options = std::make_unique<FaceLandmarkerOptions>();
    options->base_options.model_asset_path = "upstream/mediapipe_cc_lib/cc_lib/face_landmarker.task";
    options->running_mode = RunningMode::LIVE_STREAM;
    options->output_face_blendshapes = true;
    options->output_facial_transformation_matrixes = true;
    options->num_faces = 1;
    options->result_callback = [&](std::optional<FaceLandmarkerResult> result, int64_t timestamp_ms) {
        if (!result.has_value() || result->face_landmarks.size() == 0 || !result->face_blendshapes.has_value() ||
            !result->facial_transformation_matrixes.has_value()) {
            return;
        }
        auto frontend = _backend->getFrontend();
        if (!frontend) {
            return;
        }

        auto &lm = result->face_landmarks[0].landmarks;
        float lm_array[lm.size() * 3];
        float *ptr = lm_array;
        for (size_t i = 0; i < lm.size(); ++i) {
            *(ptr++) = lm[i].x;
            *(ptr++) = lm[i].y;
            *(ptr++) = lm[i].z;
        }
        std::span landmarks{lm_array, lm.size() * 3zu};
    
        auto &bs = result->face_blendshapes->at(0).categories;
    
        if (!blendshapeNamesHaveBeenSend) {
            std::vector<std::string_view> faceBlendshapeNames;
            faceBlendshapeNames.reserve(bs.size());
            for (auto &cat : bs) {
                faceBlendshapeNames.emplace_back(*cat.category_name);
            }
            frontend->faceBlendshapeNames(faceBlendshapeNames);
            blendshapeNamesHaveBeenSend = true;
        }
    
        float bs_array[bs.size()];
        ptr = bs_array;
        for (size_t i = 0; i < bs.size(); ++i) {
            *(ptr++) = bs[i].score;
        }
        std::span blendshapes{bs_array, bs.size()};
        frontend->faceLandmarks(landmarks, blendshapes, result->facial_transformation_matrixes->at(0).data, timestamp_ms);
    };
    mp = FaceLandmarker::Create(std::move(options));
}

CORBA::async<std::string> MediapipeFace::name() { co_return "face"; }

void MediapipeFace::frame(const cv::Mat &frame, int64_t timestamp_ms) {
    mp->DetectAsync(frame.channels(), frame.cols, frame.rows, frame.step, frame.data, timestamp_ms);
}
