#include "player.hh"
#include "../backend_impl.hh"
#include "../recorder/recorderdevice.hh"
#include <string>
#include <print>

using namespace std;

// FIXME: the time does not start => write a unit test for the timer and fix it
// TODO : is there an API to show all libev watcher's? no.
MoCapPlayer::MoCapPlayer(struct ev_loop *loop, const std::string &filename)
    : timer(loop, 0.0, 1.0 / 30.0,
            [this] {
                this->tick();
            }),
      mocap(FreeMoCap(filename))
{
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
        // println("MoCapPlayer::tick(): sending pose landmarks {}", pos);

        float face[53];
        float pose[165];
        float lhand[63];
        float rhand[63];

        memset(face, 0, sizeof(face));
        memset(pose, 0, sizeof(pose));
        memset(lhand, 0, sizeof(lhand));
        memset(rhand, 0, sizeof(rhand));

        auto mp = mocap[pos];
        for(size_t i=0, o=0; o<165; ) {
            pose[o++] = mp.landmarks[i++];
            pose[o++] = mp.landmarks[i++];
            pose[o++] = mp.landmarks[i++];
            pose[o++] = 1;
            pose[o++] = 1;
        }

        RecorderDevice::instance->landmarks(face, pose, lhand, rhand, pos);
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
