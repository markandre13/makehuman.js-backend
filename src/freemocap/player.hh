#pragma once

#include "../ev/timer.hh"
#include "freemocap.hh"

class Backend_impl;

/**
 * the goal of the MoCapPlayer is this:
 *
 * * return it as an object to the frontend?
 *   later, just start by letting it being controlled by the backend api we already have
 *   otherwise i might run into trouble with the corba resource management...
 * * move loading from the FreeMoCap file into another class?
 * * provide play,pause,seek to the frontend
 */
class MoCapPlayer {
    private:
        Timer timer;
        MoCap mocap;
        Backend_impl *backend;
        size_t pos = 0;
        bool paused = false;

    public:
        MoCapPlayer(struct ev_loop *loop, const std::string_view &filename, Backend_impl *backend);
        ~MoCapPlayer();
        void play();
        void pause();
        void seek(uint64_t timestamp_ms);

    private:
        void tick();
};
