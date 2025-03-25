#include "player.hh"
#include "../makehuman_impl.hh"
#include <string>
#include <print>

using namespace std;

// FIXME: the time does not start => write a unit test for the timer and fix it
// TODO : is there an API to show all libev watcher's? no.
MoCapPlayer::MoCapPlayer(struct ev_loop *loop, const std::string_view &filename, Backend_impl *backend)
    : timer(loop, 0.0, 1.0 / 30.0,
            [this] {
                this->tick();
            }),
      mocap(FreeMoCap(string(filename))),
      backend(backend) {
    println("MoCapPlayer::MoCapPlayer()");
}

MoCapPlayer::~MoCapPlayer() { println("MoCapPlayer::~MoCapPlayer()"); }

void MoCapPlayer::tick() {
    // println("MoCapPlayer::tick(): paused={}", paused);
    if (!paused) {
        ++pos;
        if (pos >= mocap.size()) {
            pos = 0;
        }
        backend->poseLandmarks(mocap[pos], pos);
    }
}
void MoCapPlayer::play() { paused = false; }
void MoCapPlayer::pause() { paused = true; }
void MoCapPlayer::seek(uint64_t timestamp_ms) {
    pause();
    if (timestamp_ms >= mocap.size()) {
        timestamp_ms = mocap.size() - 1;
    }
    this->pos = timestamp_ms;
    tick();
}
