#include "face.hh"

#include <sys/socket.h>

#include <print>
#include <string>

using namespace std;

MediapipePyFaceDevice::MediapipePyFaceDevice(struct ev_loop* loop, unsigned port) : UDPServer(loop, port) {
    println("MediapipePyFaceDevice listening on udp port {}", port);
}

std::string MediapipePyFaceDevice::id() { return "MediapipePyFaceDevice"; }
CaptureDeviceType MediapipePyFaceDevice::type() { return CaptureDeviceType::FACE; }
std::string MediapipePyFaceDevice::name() { return "Mediapipe (Python) Face"; }

struct X {
        float weights[53];
        float spacer;
        float transform[16];
        uint64_t timestamp_ms;
};

void MediapipePyFaceDevice::read() {
    unsigned char buffer[8192];
    auto fbuffer = reinterpret_cast<X*>(buffer);
    ssize_t nbytes = ::recv(fd, buffer, sizeof(buffer), 0);
    if (nbytes > 0) {
        // println("mediapype: received {} bytes", nbytes);
        // hexdump(buffer, nbytes);
        try {
            // println("got {} bytes {}", nbytes, fbuffer->timestamp_ms);
            if (_receiver) {
                _receiver->faceLandmarks(fbuffer->weights, fbuffer->transform, fbuffer->timestamp_ms);
            }
        } catch (exception& ex) {
            println("MediapipePyFaceDevice::read(): {}", ex.what());
        }
    } else {
        // println("recv -> {}", nbytes);
        if (nbytes < 0) {
            perror("MediapipePyFaceDevice::read(): recv");
        }
    }
}
