#include "recorder_impl.hh"
#include "opencv/loop.hh"
#include "opencv/videoreader.hh"

using namespace std;

CORBA::async<VideoSize> Recorder_impl::open(const std::string_view& filename) {
    println("Recorder_impl::open(\"{}\")", filename);
    string aFilename(filename);

    VideoSize size = co_await _loop->execute<VideoSize>([&] {
        auto reader = make_shared<VideoReader>(aFilename);
        _loop->setVideoReader(reader);
        VideoSize size = {
            .fps = reader->fps(),
            .frames = reader->frameCount()
        }; 
        println("Recorder_impl::open(\"{}\") -> fps={}, frames={}", aFilename, size.fps, size.frames);
        return size;
    });

    co_return size;
}
CORBA::async<> Recorder_impl::close() { co_return; }

CORBA::async<void> Recorder_impl::record() {
    println("Backend_impl::record())");
    // _stop();
    // _videoWriter = make_shared<VideoWriter>(filename);
    co_return;
}

CORBA::async<void> Recorder_impl::play() {
    println("Backend_impl::play()");

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
    println("Recorder_impl::stop()");
    // _stop();
    co_return;
}

CORBA::async<void> Recorder_impl::pause() {
    println("Recorder_impl::pause()");
    // if (mocapPlayer) {
    //     mocapPlayer->pause();
    // }
    co_return;
};
CORBA::async<void> Recorder_impl::seek(uint32_t frame) {
    // println("Recorder_impl::seek({})", frame);
    co_await _loop->execute<int>([=] {
        // println("Recorder_impl::seek({}): in opencv", frame);
        if (_reader && _reader->tell() != frame) {
            println("Recorder_impl::seek({}): actually seek", frame);
            _reader->seek(frame);
            _loop->resume();
        } else {
            // println("Recorder_impl::seek({}): already there, frontend should not request seek", frame);
        }
        return 0;
    });

    // if (mocapPlayer) {
    //     mocapPlayer->seek(timestamp_ms);
    // }
    co_return;
};
