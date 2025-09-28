#include "backend_impl.hh"

#include <ev.h>
#include <sys/socket.h>

#include <glm/ext/matrix_transform.hpp>  // glm::translate, glm::rotate, glm::scale
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>  // glm::mat4
#include <glm/vec4.hpp>    // glm::vec4

#include "ev/udpserver.hh"
#include "livelink/livelinkframe.hh"
#include "macos/video/videocamera_impl.hh"
#include "opencv/loop.hh"
#include "recorder_impl.hh"
#include "util.hh"

using namespace std;

template <typename E>
auto as_int(E const value) -> typename std::underlying_type<E>::type {
    return static_cast<typename std::underlying_type<E>::type>(value);
}

// hack to send number of current frame to backend
Backend_impl *Xbackend;
ev_async Xwatcher;
int32_t XframeNumber;
struct ev_loop *Xloop;
static void Xcallback(struct ev_loop *loop, struct ev_async *watcher, int revents) {
    // println("Xcallback {} {}", XframeNumber, revents);
    auto frontend = Xbackend->getFrontend();
    if (frontend) {
        frontend->frame(XframeNumber);
    }
}

void XinitAsync() {
    ev_async_init(&Xwatcher, &Xcallback);
    ev_async_start(Xloop, &Xwatcher);
}

Backend_impl::Backend_impl(std::shared_ptr<CORBA::ORB> orb, struct ev_loop *loop, OpenCVLoop *openCVLoop)
    : _loop(loop), openCVLoop(openCVLoop), cameras(::getVideoCameras(orb)), mediaPipeTasks(::getMediaPipeTasks(orb, this)) {
    Xloop = loop;
    Xbackend = this;

    _recorder = make_shared<Recorder_impl>(openCVLoop);
    orb->activate_object(_recorder);

    // when the openCVLoop delivers a frame, forward it to the _mediaPipeTask
    openCVLoop->frameHandler = [&](const cv::Mat &frameImage, int32_t frameNumber) {
        // mediapipe is threaded by itself and doesn't seem to mind from which thread it's called
        if (_mediaPipeTask) {
            _mediaPipeTask->frame(frameImage, getMilliseconds());
        }
        // this run from within the opencv thread, no problem here
        if (_videoWriter) {
            _videoWriter->frame(frameImage, _camera->fps());
        }

        // hack to send number of current frame to backend
        XframeNumber = frameNumber;
        ev_async_send(_loop, &Xwatcher);
        // because: neither the mutex in corba.cc nor the approach below worked
        //          without causing segfault nor adding extra bytes at the beginning
        //          of packets send via wslay.
        //          the following at least crashed once
        //          maybe using std::atomic_load(&frontend); does the trick???
        // openCVLoop->executeLibEV([=] {
        //     if (frontend) {
        //         frontend->frame(frameNumber);
        //     }
        // }).no_wait();
    };
}

CORBA::async<> Backend_impl::setFrontend(std::shared_ptr<Frontend> aFrontend) {
    println("Backend_impl::setFrontend(...)");
    atomic_store(&frontend, aFrontend);
    // frontend = aFrontend;
    CORBA::installSystemExceptionHandler(aFrontend, [this] {
        println("caught system exception in frontend stub, dropping reference");
        frontend = nullptr;
        // std::atomic_store(&frontend, std::make_shared<Frontend>(nullptr));
        println("frontend reference dropped");
    });
    blendshapeNamesHaveBeenSend = false;
    co_return;
}

class ARKitFaceDevice_impl : public virtual ARKitFaceDevice_skel {
    protected:
        std::shared_ptr<ARKitFaceReceiver> _receiver;

    public:
        CORBA::async<std::shared_ptr<ARKitFaceReceiver>> receiver() override;
        CORBA::async<void> receiver(std::shared_ptr<ARKitFaceReceiver>) override;
};

CORBA::async<std::shared_ptr<ARKitFaceReceiver>> ARKitFaceDevice_impl::receiver() { co_return _receiver; }
CORBA::async<void> ARKitFaceDevice_impl::receiver(std::shared_ptr<ARKitFaceReceiver> receiver) {
    _receiver = receiver;
    co_return;
}

class LiveLinkFaceDevice : public virtual ARKitFaceDevice_impl, private UDPServer {
    public:
        LiveLinkFaceDevice(struct ev_loop *loop, unsigned port);
        CORBA::async<void> receiver(std::shared_ptr<ARKitFaceReceiver>) override;
        virtual CORBA::async<CaptureDeviceType> type() override;
        virtual CORBA::async<std::string> name() override;

    private:
        bool _blendshapeNamesHaveBeenSend;
        void read() override;
};

LiveLinkFaceDevice::LiveLinkFaceDevice(struct ev_loop *loop, unsigned port) : UDPServer(loop, port), _blendshapeNamesHaveBeenSend(false) {}

CORBA::async<void> LiveLinkFaceDevice::receiver(std::shared_ptr<ARKitFaceReceiver> receiver) {
    co_await ARKitFaceDevice_impl::receiver(receiver);
    _blendshapeNamesHaveBeenSend = false;
    co_return;
}
CORBA::async<CaptureDeviceType> LiveLinkFaceDevice::type() { co_return CaptureDeviceType::FACE; }
CORBA::async<std::string> LiveLinkFaceDevice::name() { co_return "Live Link Face"; }

void LiveLinkFaceDevice::read() {
    unsigned char buffer[8192];
    ssize_t nbytes = ::recv(fd, buffer, sizeof(buffer), 0);
    if (nbytes > 0) {
        // println("livelink: received {} bytes", nbytes);
        // hexdump(buffer, nbytes);
        try {
            LiveLinkFrame frame(buffer, nbytes);
            // callback(frame);
            if (_receiver) {
                if (!_blendshapeNamesHaveBeenSend) {
                    _receiver->faceBlendshapeNames(LiveLinkFrame::blendshapeNames);
                    _blendshapeNamesHaveBeenSend = true;
                }
                const size_t headYaw = 52;    // Y
                const size_t headPitch = 53;  // X
                const size_t headRoll = 54;   // Z

                auto m = glm::identity<glm::mat4x4>();
                m = glm::rotate(m, frame.weights[headRoll], glm::vec3(0.0f, 0.0f, 1.0f));
                m = glm::rotate(m, frame.weights[headPitch], glm::vec3(-1.0f, 0.0f, 0.0f));
                m = glm::rotate(m, frame.weights[headYaw], glm::vec3(0.0f, 1.0f, 0.0f));
                // m = glm::translate(m, glm::vec3(0.0f, 0.0f, -20));
                auto transform = span(const_cast<float *>(glm::value_ptr(m)), 16);
                _receiver->faceLandmarks({}, frame.weights, transform, frame.frame);
            }
        } catch (exception &ex) {
            println("LiveLink::read(): {}", ex.what());
        }
    } else {
        // println("recv -> {}", nbytes);
        if (nbytes < 0) {
            perror("recv");
        }
    }
}

CORBA::async<std::vector<std::shared_ptr<CaptureDevice>>> Backend_impl::captureDevices() {
    if (_captureDevices.size() == 0) {
        auto ll = make_shared<LiveLinkFaceDevice>(_loop, 11111);
        _captureDevices.push_back(std::static_pointer_cast<CaptureDevice>(ll));
    }
    co_return _captureDevices;
}

CORBA::async<std::shared_ptr<Recorder>> Backend_impl::recorder() { co_return _recorder; }

/*
 * Select Videocamera
 */
CORBA::async<std::vector<std::shared_ptr<VideoCamera>>> Backend_impl::getVideoCameras() { co_return cameras; }
CORBA::async<std::shared_ptr<VideoCamera>> Backend_impl::camera() { co_return _camera; }
CORBA::async<> Backend_impl::camera(std::shared_ptr<VideoCamera> camera) {
    auto impl = dynamic_pointer_cast<VideoCamera_impl>(camera);
    if (camera && !impl) {
        println("ERROR: Backend_impl::setCamera(camera): provided camera is not an instance of VideoCamera_impl");
    }
    _camera = impl;
    openCVLoop->setCamera(impl);
    co_return;
}

/*
 * Select Mediapipe Task
 */
CORBA::async<vector<shared_ptr<MediaPipeTask>>> Backend_impl::getMediaPipeTasks() { co_return mediaPipeTasks; }
CORBA::async<shared_ptr<MediaPipeTask>> Backend_impl::mediaPipeTask() { co_return _mediaPipeTask; }
CORBA::async<> Backend_impl::mediaPipeTask(shared_ptr<MediaPipeTask> task) {
    _mediaPipeTask = dynamic_pointer_cast<MediaPipeTask_impl>(task);
    co_return;
}

/*
 *
 * OLD OBSOLETE API AND API'S TO BE REVIEWED
 *
 */

// CORBA::async<> Backend_impl::setEngine(MotionCaptureType type, MotionCaptureEngine engine) {
//     println("Backend_impl::setEngine(...)");
//     switch (type) {
//         case MotionCaptureType::FACE:
//             blendshapeNamesHaveBeenSend = false;
//             switch (engine) {
//                 case MotionCaptureEngine::NONE:
//                     // face = nullptr;
//                     face.reset();
//                     break;
//                 case MotionCaptureEngine::MEDIAPIPE:
//                     // face = nullptr;
//                     face.reset();
//                     // TODO: this is currently always on...
//                     // captureEngine = new MediaPipe(...);
//                     break;
//                 case MotionCaptureEngine::LIVELINK:
//                     // NOTE: loop runs in it's own thread... but CORBA is on the same thread
//                     //       so adding a udp listener here should work
//                     face = make_unique<LiveLink>(loop, 11111, [&](const LiveLinkFrame &frame) {
//                         // println("frame {}.{}", frame.frame, frame.subframe);
//                         this->livelink(const_cast<LiveLinkFrame &>(frame));
//                         // metalRenderer->faceLandmarks(frame);
//                     });
//                     break;
//                 default:;
//             }
//             break;
//         case MotionCaptureType::BODY:
//             switch (engine) {
//                 case MotionCaptureEngine::NONE:
//                     // face = nullptr;
//                     body.reset();
//                     break;
//                 case MotionCaptureEngine::MEDIAPIPE:
//                     // face = nullptr;
//                     body.reset();
//                     // captureEngine = new MediaPipe(...);
//                     break;
//                 default:;
//             }
//             break;
//         default:
//             println("Backend::setEngine(type={}, engine={}): not possible or implemented", as_int(type), as_int(engine));
//     }
//     co_return;
// }

void Backend_impl::chordata(const char *buffer, size_t nbytes) {
    std::shared_ptr<Frontend> fe = std::atomic_load(&this->frontend);
    if (!fe) {
        return;
    }
    fe->chordata(CORBA::blob_view(buffer, nbytes));
}

void Backend_impl::poseLandmarks(const BlazePose &pose, int64_t timestamp_ms) {
    std::shared_ptr<Frontend> fe = std::atomic_load(&this->frontend);
    if (!fe) {
        return;
    }

    BlazePose a(pose);  // WTF???
    // float &lm_array[33 * 3] {pose.landmarks};
    // std::span<float> landmarks{span(*pose.landmarks, sizeof(pose.landmarks)};

    std::span<float> landmarks(a.landmarks, 99);
    fe->poseLandmarks(landmarks, timestamp_ms);
}

/*
 *
 * record video
 *
 */

// TODO: can we improve the timer api

void Backend_impl::_stop() {
    if (_videoWriter) {
        println("Backend_impl::_stop(): stop VideoWriter");
        _videoWriter = nullptr;
    }
    if (videoReader) {
        println("Backend_impl::_stop(): stop VideoReader");
        openCVLoop->setVideoReader(nullptr);
        videoReader = nullptr;
    }
    if (mocapPlayer) {
        println("Backend_impl::_stop(): stop MoCapPlayer");
        mocapPlayer = nullptr;
    }
}

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
    std::shared_ptr<VideoWriter> out = std::atomic_load(&this->_videoWriter);
    if (!out) {
        return;
    }
    out->frame(frame, fps);
}
