
#include "backend_impl.hh"
#include <ev.h>

#include "recorder/recorder_impl.hh"
#include "recorder/recorderdevice.hh"
#include "freemocap/freemocap.hh"
#include "livelink/livelink.hh"
#include "mediapipe-py/face.hh"
#include "mediapipe-py/holistic.hh"

#include "macos/video/videocamera_impl.hh"

#include "opencv/loop.hh"
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

    auto rd = make_shared<RecorderDevice>(_loop);
    orb->activate_object(rd);
    _captureDevices.push_back(std::static_pointer_cast<LocalCaptureDevice>(rd));

    auto ll = make_shared<LiveLinkFaceDevice>(_loop, 11111);
    orb->activate_object(ll);
    _captureDevices.push_back(std::static_pointer_cast<LocalCaptureDevice>(ll));

    auto mf = make_shared<MediapipePyFaceDevice>(_loop, 11110);
    orb->activate_object(mf);
    _captureDevices.push_back(std::static_pointer_cast<LocalCaptureDevice>(mf));

    auto mh = make_shared<MediapipePyHolisticDevice>(_loop, 11112);
    orb->activate_object(mh);
    _captureDevices.push_back(std::static_pointer_cast<LocalCaptureDevice>(mh));

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

CORBA::async<std::vector<CaptureDeviceInfo>> Backend_impl::captureDevices() {
    std::vector<CaptureDeviceInfo> result;
    for (auto &device : _captureDevices) {
        result.push_back({
            .device = device,
            .type = device->type(),
            .id = device->id(),
            .name = device->name()
        });
    }
    co_return result;
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

void Backend_impl::chordata(const char *buffer, size_t nbytes) {
    std::shared_ptr<Frontend> fe = std::atomic_load(&this->frontend);
    if (!fe) {
        return;
    }
    fe->chordata(CORBA::blob_view(buffer, nbytes));
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
