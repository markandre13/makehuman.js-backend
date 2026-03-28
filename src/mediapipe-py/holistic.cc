#include "holistic.hh"

#include <sys/socket.h>

#include <print>
#include <string>

using namespace std;

MediapipePyHolisticDevice::MediapipePyHolisticDevice(struct ev_loop* loop, unsigned port) : UDPServer(loop, port) {
    println("MediapipePyHolisticDevice listening on udp port {}", port);
}

std::string MediapipePyHolisticDevice::id() { return "MediapipePyHolisticDevice"; }
CaptureDeviceType MediapipePyHolisticDevice::type() { return CaptureDeviceType::HOLISTIC; }
std::string MediapipePyHolisticDevice::name() { return "Mediapipe (Python) Holistic"; }

struct X {
        float face[53];
        float pose[165];
        float lhand[63];
        float rhand[63];
        uint64_t timestamp_ms;
};

void MediapipePyHolisticDevice::read() {
    unsigned char buffer[8192];
    auto fbuffer = reinterpret_cast<X*>(buffer);
    ssize_t nbytes = ::recv(fd, buffer, sizeof(buffer), 0);
    if (nbytes > 0) {
        // println("mediapype: received {} bytes", nbytes);
        // hexdump(buffer, nbytes);
        try {
            // println("got {} bytes {}", nbytes, fbuffer->timestamp_ms);
            if (_receiver) {
                _receiver->landmarks(fbuffer->face, fbuffer->pose, fbuffer->lhand, fbuffer->rhand, fbuffer->timestamp_ms);
            }
        } catch (exception& ex) {
            println("MediapipePyHolisticDevice::read(): {}", ex.what());
        }
    } else {
        // println("recv -> {}", nbytes);
        if (nbytes < 0) {
            perror("MediapipePyHolisticDevice::read(): recv");
        }
    }
}
