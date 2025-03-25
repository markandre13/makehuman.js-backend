#include "recorder_impl.hh"
#include "opencv/videoreader.hh"

using namespace std;

CORBA::async<VideoSize> Recorder_impl::open(const std::string_view& filename) {
    println("Recorder_impl::open(\"{}\")", filename);
    auto videoReader = make_shared<VideoReader>(filename);
    VideoSize size = {
        .fps = videoReader->fps(),
        .frames = videoReader->frameCount()
    }; 
    co_return size;
}
CORBA::async<> Recorder_impl::close() { co_return; }

CORBA::async<void> Recorder_impl::record() {
    // println("Backend_impl::record(\"{}\")", filename);
    // _stop();
    // _videoWriter = make_shared<VideoWriter>(filename);
    co_return;
}

CORBA::async<void> Recorder_impl::play() {
    // println("Backend_impl::play(\"{}\")", filename);

    // videoReader = make_shared<VideoReader>(filename);
    // openCVLoop->setVideoReader(videoReader);

    // if (mocapPlayer) {
    //     mocapPlayer->play();
    // } else {
    //     _stop();
    //     if (filename.ends_with(".csv")) {
    //         println("Backend_impl::play(\"{}\"): create MoCapPlayer", filename);
    //         mocapPlayer = make_shared<MoCapPlayer>(loop, filename, this);
    //     }
    //     if (filename.ends_with(".mp4")) {
    //         videoReader = make_shared<VideoReader>(filename);
    //     }
    // }
    // co_return VideoRange{
    //     .fps = static_cast<uint16_t>(videoReader->fps()),
    //     .firstFrame = 0,
    //     .lastFrame = static_cast<uint32_t>(videoReader->frameCount())
    // };
    co_return;
}

CORBA::async<void> Recorder_impl::stop() {
    // _stop();
    co_return;
}

CORBA::async<void> Recorder_impl::pause() {
    // println("Backend_impl::pause()");
    // if (mocapPlayer) {
    //     mocapPlayer->pause();
    // }
    co_return;
};
CORBA::async<void> Recorder_impl::seek(uint32_t frame) {
    println("Recorder_impl::seek({})", frame);

    // if (mocapPlayer) {
    //     mocapPlayer->seek(timestamp_ms);
    // }
    co_return;
};
