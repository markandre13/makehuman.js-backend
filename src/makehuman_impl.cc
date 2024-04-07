#include "makehuman_impl.hh"

#include <corba/orb.hh>
#include <print>

Backend_impl::Backend_impl(std::shared_ptr<CORBA::ORB> orb) : Backend_skel(orb) {}

CORBA::async<> Backend_impl::setFrontend(std::shared_ptr<Frontend> aFrontend) {
    std::println("set frontend: enter");
    frontend = aFrontend;
    CORBA::installSystemExceptionHandler(aFrontend, [this] {
        std::println("caught system exception in frontend stub, dropping reference");
        this->frontend = nullptr;
        std::println("frontend reference dropped");
    });
    blendshapeNamesHaveBeenSend = false;
    std::println("set frontend: leave");
    co_return;
}

CORBA::async<> Backend_impl::setEngine(MotionCaptureEngine engine, MotionCaptureType type, EngineStatus status) {
    std::println("SET ENGINE engine, type, status");
    co_return;
}

void Backend_impl::faceLandmarks(std::optional<mediapipe::cc_lib::vision::face_landmarker::FaceLandmarkerResult> result, int64_t timestamp_ms) {
    if (!frontend) {
        return;
    }
    if (!result.has_value() || result->face_landmarks.size() == 0) {
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

    frontend->faceLandmarks(landmarks, blendshapes, timestamp_ms);
}