#include "recorder_impl.hh"

#include "../freemocap/player.hh"
#include "../opencv/loop.hh"
#include "../opencv/videoreader.hh"
#include "recorderdevice.hh"

using namespace std;

CORBA::async<VideoSize> Recorder_impl::open(const std::string_view& filenameX) {
    string filename("/Users/mark/freemocap_data/recording_sessions/session_2024-10-06_13_24_28/recording_13_29_02_gmt+2__drei/output_data/mediapipe_body_3d_xyz.csv");

    println("Recorder_impl::open(\"{}\")", filename);

    if (filename.ends_with(".csv")) {
        println("Backend_impl::play(\"{}\"): create MoCapPlayer", filename);
        mocapPlayer = make_shared<MoCapPlayer>(_ev_loop, filename);
        // new MoCapPlayer(_ev_loop, filename);
    }
    VideoSize size = {.fps = 24, .frames = 0};
    if (filename.ends_with(".mp4")) {
        //     videoReader = make_shared<VideoReader>(filename);
        // }
    // string aFilename(filename);
    // VideoSize size = co_await _loop->execute<VideoSize>([&] {
    //     auto reader = make_shared<VideoReader>(aFilename);
    //     _loop->setVideoReader(reader);
    //     VideoSize size = {
    //         .fps = reader->fps(),
    //         .frames = reader->frameCount()
    //     };
    //     println("Recorder_impl::open(\"{}\") -> fps={}, frames={}", aFilename, size.fps, size.frames);
    //     return size;
    // });
    }

    co_return size;
}
CORBA::async<> Recorder_impl::close() { co_return; }

CORBA::async<void> Recorder_impl::record() {
    println("Recorder_impl::record() not implemented yet");
    // _stop();
    // _videoWriter = make_shared<VideoWriter>(filename);
    co_return;
}

CORBA::async<void> Recorder_impl::play() {
    // println("Recorder_impl::play() not implemented yet");

    // videoReader = make_shared<VideoReader>(filename);
    // openCVLoop->setVideoReader(videoReader);

    string filename;

    if (mocapPlayer) {
        println("Recorder_impl::play(): mocapPlayer->play()");
        mocapPlayer->play();
    } else {
        // _stop();
        // if (filename.ends_with(".csv")) {
        //     println("Backend_impl::play(\"{}\"): create MoCapPlayer", filename);
        //     mocapPlayer = make_shared<MoCapPlayer>(_ev_loop, filename);
        //     // new MoCapPlayer(_ev_loop, filename);
        // }
        // if (filename.ends_with(".mp4")) {
        //     videoReader = make_shared<VideoReader>(filename);
        // }
    }
    // co_return VideoRange{
    //     .fps = static_cast<uint16_t>(videoReader->fps()),
    //     .firstFrame = 0,
    //     .lastFrame = static_cast<uint32_t>(videoReader->frameCount())
    // };
    co_return;
}

CORBA::async<void> Recorder_impl::stop() {
    println("Recorder_impl::stop() not implemented yet");
    // _stop();
    co_return;
}

CORBA::async<void> Recorder_impl::pause() {
    println("Recorder_impl::pause() not implemented yet");
    // if (mocapPlayer) {
    //     mocapPlayer->pause();
    // }
    co_return;
};
CORBA::async<void> Recorder_impl::seek(uint32_t frame) {
    // println("Recorder_impl::seek({})", frame);
    co_await _cv_loop->execute<int>([=, this] {
        // println("Recorder_impl::seek({}): in opencv", frame);
        if (_reader && _reader->tell() != frame) {
            println("Recorder_impl::seek({}): actually seek", frame);
            _reader->seek(frame);
            _cv_loop->resume();
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
