#pragma once

#include "generated/makehuman_skel.hh"

class Recorder_impl : public Recorder_skel {
    CORBA::async<VideoSize> open(const std::string_view & filename) override;
    CORBA::async<> close() override;
    CORBA::async<> record() override;
    CORBA::async<> play() override;
    CORBA::async<> stop() override;
    CORBA::async<> pause() override;
    CORBA::async<> seek(uint32_t frame) override;
};