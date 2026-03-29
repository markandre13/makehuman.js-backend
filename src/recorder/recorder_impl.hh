#pragma once

#include "../generated/makehuman_skel.hh"

class OpenCVLoop;
class VideoReader;
class MoCapPlayer;

/**
 * 
 */
class Recorder_impl : public Recorder_skel {
        struct ev_loop *_ev_loop;
        OpenCVLoop *_cv_loop;
        std::shared_ptr<VideoReader> _reader;
        std::shared_ptr<MoCapPlayer> mocapPlayer;
    public:
        Recorder_impl(struct ev_loop *ev_loop, OpenCVLoop *cv_loop): _ev_loop(ev_loop), _cv_loop(cv_loop) {};
        CORBA::async<VideoSize> open(const std::string_view& filename) override;
        CORBA::async<> close() override;
        CORBA::async<> record() override;
        CORBA::async<> play() override;
        CORBA::async<> stop() override;
        CORBA::async<> pause() override;
        CORBA::async<> seek(uint32_t frame) override;
};