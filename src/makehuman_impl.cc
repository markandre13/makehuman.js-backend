#include "makehuman_impl.hh"

#include "livelink/livelink.hh"
#include "livelink/livelinkframe.hh"

#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/vec4.hpp> // glm::vec4
#include <glm/ext/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale
#include <glm/gtc/type_ptr.hpp>

#include <atomic>
#include <corba/orb.hh>
#include <print>

using namespace std;

Backend_impl::Backend_impl(std::shared_ptr<CORBA::ORB> orb, struct ev_loop *loop) : Backend_skel(orb), loop(loop) {}

template <typename E>
auto as_int(E const value) -> typename std::underlying_type<E>::type {
    return static_cast<typename std::underlying_type<E>::type>(value);
}

CORBA::async<> Backend_impl::setFrontend(std::shared_ptr<Frontend> aFrontend) {
    std::println("set frontend: enter");
    std::atomic_store(&frontend, aFrontend);
    // frontend = aFrontend;
    CORBA::installSystemExceptionHandler(aFrontend, [this] {
        std::println("caught system exception in frontend stub, dropping reference");
        frontend = nullptr;
        // std::atomic_store(&frontend, std::make_shared<Frontend>(nullptr));
        std::println("frontend reference dropped");
    });
    blendshapeNamesHaveBeenSend = false;
    std::println("set frontend: leave");
    co_return;
}

CORBA::async<> Backend_impl::setEngine(MotionCaptureType type, MotionCaptureEngine engine) {
    switch(type) {
        case MotionCaptureType::FACE:
            blendshapeNamesHaveBeenSend = false;
            switch(engine) {
                case MotionCaptureEngine::NONE:
                    // face = nullptr;
                    face.reset();
                    break;
                case MotionCaptureEngine::MEDIAPIPE:
                    // face = nullptr;
                    face.reset();
                    // captureEngine = new MediaPipe(...);
                    break;
                case MotionCaptureEngine::LIVELINK:
                    // NOTE: loop runs in it's own thread... but CORBA is on the same thread
                    //       so adding a udp listener here should work
                    face = make_unique<LiveLink>(loop, 11111, [&](const LiveLinkFrame &frame) {
                        // println("frame {}.{}", frame.frame, frame.subframe);
                        this->livelink(const_cast<LiveLinkFrame &>(frame));
                        // metalRenderer->faceLandmarks(frame);
                    });
                    break;
                default:
                    ;
            }
            break;
        default:
            std::println("Backend::setEngine(type={}, engine={}): not possible or implemented", as_int(type), as_int(engine));
    }
    co_return;
}

void Backend_impl::chordata(const char *buffer, size_t nbytes) {
    std::shared_ptr<Frontend> fe = std::atomic_load(&this->frontend);
    if (!fe) {
        return;
    }
    fe->chordata(CORBA::blob_view(buffer, nbytes));
}

void Backend_impl::livelink(LiveLinkFrame &frame) {
    if (!blendshapeNamesHaveBeenSend) {
        frontend->faceBlendshapeNames(LiveLinkFrame::blendshapeNames);
        blendshapeNamesHaveBeenSend = true;
    }
    const size_t headYaw = 52; // Y
    const size_t headPitch = 53; // X
    const size_t headRoll = 54; // Z

    auto m = glm::identity<glm::mat4x4>();
    m = glm::rotate(m, frame.weights[headRoll], glm::vec3(0.0f, 0.0f, 1.0f));
    m = glm::rotate(m, frame.weights[headPitch], glm::vec3(-1.0f, 0.0f, 0.0f));
    m = glm::rotate(m, frame.weights[headYaw], glm::vec3(0.0f, 1.0f, 0.0f));
    // m = glm::translate(m, glm::vec3(0.0f, 0.0f, -20));
    auto transform = span(const_cast<float*>(glm::value_ptr(m)), 16);

    std::shared_ptr<Frontend> fe = std::atomic_load(&this->frontend);
    if (!fe) {
        return;
    }
    fe->faceLandmarks({}, frame.weights, transform, frame.frame);
}

#ifdef HAVE_MEDIAPIPE

void Backend_impl::faceLandmarks(std::optional<mediapipe::cc_lib::vision::face_landmarker::FaceLandmarkerResult> result, int64_t timestamp_ms) {
    if (face != nullptr) {
        return;
    }
    if (!result.has_value() || result->face_landmarks.size() == 0 || !result->face_blendshapes.has_value() ||
        !result->facial_transformation_matrixes.has_value()) {
        return;
    }

    std::shared_ptr<Frontend> fe = std::atomic_load(&this->frontend);
    if (!fe) {
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

    fe->faceLandmarks(landmarks, blendshapes, result->facial_transformation_matrixes->at(0).data, timestamp_ms);
}

#endif
