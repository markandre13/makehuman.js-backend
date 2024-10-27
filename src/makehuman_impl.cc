#include "makehuman_impl.hh"

#include "livelink/livelink.hh"
#include "livelink/livelinkframe.hh"


#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/vec4.hpp> // glm::vec4
#include <glm/ext/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale
#include <glm/gtc/type_ptr.hpp>

#include <corba/orb.hh>

#include <atomic>
#include <print>
#include <fstream>

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

using namespace std;

Backend_impl::Backend_impl(std::shared_ptr<CORBA::ORB> orb, struct ev_loop *loop) : Backend_skel(orb), loop(loop) {}

template <typename E>
auto as_int(E const value) -> typename std::underlying_type<E>::type {
    return static_cast<typename std::underlying_type<E>::type>(value);
}

CORBA::async<> Backend_impl::setFrontend(std::shared_ptr<Frontend> aFrontend) {
    println("set frontend: enter");
    atomic_store(&frontend, aFrontend);
    // frontend = aFrontend;
    CORBA::installSystemExceptionHandler(aFrontend, [this] {
        println("caught system exception in frontend stub, dropping reference");
        frontend = nullptr;
        // std::atomic_store(&frontend, std::make_shared<Frontend>(nullptr));
        println("frontend reference dropped");
    });
    blendshapeNamesHaveBeenSend = false;
    println("set frontend: leave");
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
                    // TODO: this is currently always on...
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
        case MotionCaptureType::BODY:
            switch(engine) {
                case MotionCaptureEngine::NONE:
                    // face = nullptr;
                    body.reset();
                    break;
                case MotionCaptureEngine::MEDIAPIPE:
                    // face = nullptr;
                    body.reset();
                    // captureEngine = new MediaPipe(...);
                    break;
                default:
                    ;
            }
            break;
        default:
            println("Backend::setEngine(type={}, engine={}): not possible or implemented", as_int(type), as_int(engine));
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

void Backend_impl::poseLandmarks(std::optional<mediapipe::cc_lib::vision::pose_landmarker::PoseLandmarkerResult> result, int64_t timestamp_ms) {
    if (!result.has_value() || result->pose_landmarks.size() == 0) {
        return;
    }

    std::shared_ptr<Frontend> fe = std::atomic_load(&this->frontend);
    if (!fe) {
        return;
    }

    // pose_landmarks      : relative to image
    // pose_world_landmarks: hip at 0,0,0 

    auto &lm = result->pose_world_landmarks[0].landmarks;
    float lm_array[lm.size() * 3];
    float *ptr = lm_array;
    for (size_t i = 0; i < lm.size(); ++i) {
        *(ptr++) = lm[i].x;
        *(ptr++) = lm[i].y;
        *(ptr++) = lm[i].z;
    }
    std::span landmarks{lm_array, lm.size() * 3zu};
    fe->poseLandmarks(landmarks, timestamp_ms);
}
#endif

void Backend_impl::poseLandmarks(const BlazePose &pose, int64_t timestamp_ms) {
    std::shared_ptr<Frontend> fe = std::atomic_load(&this->frontend);
    if (!fe) {
        return;
    }

    BlazePose a(pose); // WTF???
    // float &lm_array[33 * 3] {pose.landmarks};
    // std::span<float> landmarks{span(*pose.landmarks, sizeof(pose.landmarks)};

    std::span<float> landmarks(a.landmarks, 99);
    fe->poseLandmarks(landmarks, timestamp_ms);
}

/*
 *
 * load/save files
 * 
 */

static void checkFilename(const std::string_view &filename) {
    if (filename.size() == 0) {
        throw runtime_error("Backend_impl::save(): filename must not be empty");
    }
    if (filename[0] == '.') {
        throw runtime_error("Backend_impl::save(): filename must not start with a dot");
    }
    if (filename.find('/') != filename.npos) {
        throw runtime_error("Backend_impl::save(): filename must not contain a '/'");
    }
}

CORBA::async<> Backend_impl::save(const std::string_view &filename, const std::string_view &data) {
    println("save {}", filename);
    checkFilename(filename);
    std::ofstream file(string(filename).c_str());
    if (!file) {
        println("failed to write");
    }
    file.write(data.data(), data.size());
    // file << string(data);
    println("ok");
    co_return;
}
CORBA::async<std::string> Backend_impl::load(const std::string_view &filename) {
    println("load {}", filename);

    checkFilename(filename);

    int fd = open(filename.data(), O_RDONLY);
    if (fd < 0) {
        throw runtime_error(format("failed to open file '{}': {}", filename, strerror(errno)));
    }
    off_t len = lseek(fd, 0, SEEK_END);
    if (len < 0) {
        throw runtime_error(format("failed to get size of file '{}': {}", filename, strerror(errno)));
    }
    const char * data = (const char*)mmap(nullptr, len, PROT_READ, MAP_FILE | MAP_PRIVATE, fd, 0);
    if (data == MAP_FAILED) {
        throw runtime_error(format("failed to mmap file '{}': {}", filename, strerror(errno)));
    }

    string result(data, len);

    munmap((void*)data, len);
    close(fd);

    println("ok");

    co_return result;
}

/*
 *
 * record video
 * 
 */

CORBA::async<void> Backend_impl::record(const std::string_view & filename) {
    _stop();
    println("start record \"{}\"", filename);
    videoWriter = make_shared<VideoWriter>(filename);
    co_return;
}
CORBA::async<Range> Backend_impl::play(const std::string_view & filename) {
    _stop();
    println("start playing\"{}\"", filename);
    videoReader = make_shared<VideoReader>(filename);
    co_return Range { .start_ms = 0, .end_ms = 25};;
}
CORBA::async<void> Backend_impl::stop() {
    _stop();
    co_return;
}
void Backend_impl::_stop() {
    if (videoWriter) {
        println("stop recording");
        videoWriter = nullptr;
    }
    if (videoReader) {
        println("stop playing");
        videoReader = nullptr;
    }
}
CORBA::async<void> Backend_impl::pause() {
    co_return;
};
CORBA::async<void> Backend_impl::seek(uint64_t timestamp_ms) {
    co_return;
};

bool Backend_impl::readFrame(cv::Mat &frame) {
    std::shared_ptr<VideoReader> in = std::atomic_load(&this->videoReader);
    if (!in) {
        return false;
    }
    *in >> frame;
    return true;
}
int Backend_impl::delay() {
    std::shared_ptr<VideoReader> in = std::atomic_load(&this->videoReader);
    if (!in) {
        return 0;
    }
    return in->delay();
}
void Backend_impl::reset() {
    std::shared_ptr<VideoReader> in = std::atomic_load(&this->videoReader);
    if (!in) {
        return;
    }
    in->reset();
}

void Backend_impl::saveFrame(const cv::Mat &frame, double fps) {
    std::shared_ptr<VideoWriter> out = std::atomic_load(&this->videoWriter);
    if (!out) {
        return;
    }
    out->frame(frame, fps);
}
