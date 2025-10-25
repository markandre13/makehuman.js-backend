#include "livelink.hh"

#include <sys/socket.h>

#include <corba/util/hexdump.hh>
#include <glm/ext/matrix_transform.hpp>  // glm::translate, glm::rotate, glm::scale
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>  // glm::mat4
#include <glm/vec4.hpp>    // glm::vec4
#include <print>
#include <string>

#include "livelinkframe.hh"

using namespace std;

std::vector<std::string_view> mhBlendshapeNames{
    "neutral",              // 0
    "browDownLeft",         // 1
    "browDownRight",        // 2
    "browInnerUp",          // 3
    "browOuterUpLeft",      // 4
    "browOuterUpRight",     // 5
    "cheekPuff",            // 6
    "cheekSquintLeft",      // 7
    "cheekSquintRight",     // 8
    "eyeBlinkLeft",         // 9
    "eyeBlinkRight",        // 10
    "eyeLookDownLeft",      // 11
    "eyeLookDownRight",     // 12
    "eyeLookInLeft",        // 13
    "eyeLookInRight",       // 14
    "eyeLookOutLeft",       // 15
    "eyeLookOutRight",      // 16
    "eyeLookUpLeft",        // 17
    "eyeLookUpRight",       // 18
    "eyeSquintLeft",        // 19
    "eyeSquintRight",       // 20
    "eyeWideLeft",          // 21
    "eyeWideRight",         // 22
    "jawForward",           // 23
    "jawLeft",              // 24
    "jawOpen",              // 25
    "jawRight",             // 26
    "mouthClose",           // 27
    "mouthDimpleLeft",      // 28
    "mouthDimpleRight",     // 29
    "mouthFrownLeft",       // 30
    "mouthFrownRight",      // 31
    "mouthFunnel",          // 32
    "mouthLeft",            // 33
    "mouthLowerDownLeft",   // 34
    "mouthLowerDownRight",  // 35
    "mouthPressLeft",       // 36
    "mouthPressRight",      // 37
    "mouthPucker",          // 38
    "mouthRight",           // 39
    "mouthRollLower",       // 40
    "mouthRollUpper",       // 41
    "mouthShrugLower",      // 42
    "mouthShrugUpper",      // 43
    "mouthSmileLeft",       // 44
    "mouthSmileRight",      // 45
    "mouthStretchLeft",     // 46
    "mouthStretchRight",    // 47
    "mouthUpperUpLeft",     // 48
    "mouthUpperUpRight",    // 49
    "noseSneerLeft",        // 50
    "noseSneerRight",       // 51
    "tongueOut",            // 52
};

/**
 * map array of weights from live link face to makehuman.js
 */
class LL2MH {
        std::vector<size_t> makeHuman2liveLink;
    public:
        LL2MH();
        void convert(const LiveLinkFrame &frame, float weights[53]);
};

LL2MH::LL2MH() : makeHuman2liveLink(53, 0) {
    // sort weights to fit enum in makehuman.js
    std::map<string_view, size_t> liveLinkNameToIndex;
    size_t idx = 0;
    for (const auto& l : LiveLinkFrame::blendshapeNames) {
        liveLinkNameToIndex[l] = idx;
        ++idx;
    }
    idx = 0;
    for (const auto& m : mhBlendshapeNames) {
        auto p = liveLinkNameToIndex.find(m);
        if (p != liveLinkNameToIndex.end()) {
            makeHuman2liveLink[idx] = p->second;
        }
        ++idx;
    }
}

void LL2MH::convert(const LiveLinkFrame &frame, float *weights) {
    for (size_t i = 1; i < 53; ++i) {
        weights[i] = frame.weights[makeHuman2liveLink[i]];
    }
}

static LL2MH ll2mh;

LiveLinkFaceDevice::LiveLinkFaceDevice(struct ev_loop* loop, unsigned port) : UDPServer(loop, port), _blendshapeNamesHaveBeenSend(false) {
    println("LiveLinkFaceDevice listening on udp port {}", port);
}

CORBA::async<void> LiveLinkFaceDevice::receiver(std::shared_ptr<ARKitFaceReceiver> receiver) {
    // co_await ARKitFaceDevice_impl::receiver(receiver);
    _receiver = receiver;
    _blendshapeNamesHaveBeenSend = false;
    co_return;
}

std::string LiveLinkFaceDevice::id() { return "xxx"; }
CaptureDeviceType LiveLinkFaceDevice::type() { return CaptureDeviceType::FACE; }
std::string LiveLinkFaceDevice::name() { return "Life Link Face"; }

void LiveLinkFaceDevice::read() {
    unsigned char buffer[8192];
    ssize_t nbytes = ::recv(fd, buffer, sizeof(buffer), 0);
    if (nbytes > 0) {
        // println("livelink: received {} bytes", nbytes);
        // hexdump(buffer, nbytes);
        try {
            if (_receiver) {
                LiveLinkFrame frame(buffer, nbytes);

                float weights[53];
                ll2mh.convert(frame, weights);

                // create transform matrix similar to mediapipe
                const size_t headYaw = 52;    // Y
                const size_t headPitch = 53;  // X
                const size_t headRoll = 54;   // Z
                auto m = glm::identity<glm::mat4x4>();
                m = glm::rotate(m, frame.weights[headRoll], glm::vec3(0.0f, 0.0f, 1.0f));
                m = glm::rotate(m, frame.weights[headPitch], glm::vec3(-1.0f, 0.0f, 0.0f));
                m = glm::rotate(m, frame.weights[headYaw], glm::vec3(0.0f, 1.0f, 0.0f));
                auto transform = span(const_cast<float*>(glm::value_ptr(m)), 16);
                _receiver->faceLandmarks(weights, transform, frame.frame);
            }
        } catch (exception& ex) {
            println("LiveLink::read(): {}", ex.what());
        }
    } else {
        // println("recv -> {}", nbytes);
        if (nbytes < 0) {
            perror("LiveLink::read(): recv");
        }
    }
}
