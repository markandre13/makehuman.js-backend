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

LiveLinkFaceDevice::LiveLinkFaceDevice(struct ev_loop *loop, unsigned port) : UDPServer(loop, port), _blendshapeNamesHaveBeenSend(false) {
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
            LiveLinkFrame frame(buffer, nbytes);
            // callback(frame);
            if (_receiver) {
                if (!_blendshapeNamesHaveBeenSend) {
                    _receiver->faceBlendshapeNames(LiveLinkFrame::blendshapeNames);
                    _blendshapeNamesHaveBeenSend = true;
                }
                const size_t headYaw = 52;    // Y
                const size_t headPitch = 53;  // X
                const size_t headRoll = 54;   // Z

                auto m = glm::identity<glm::mat4x4>();
                m = glm::rotate(m, frame.weights[headRoll], glm::vec3(0.0f, 0.0f, 1.0f));
                m = glm::rotate(m, frame.weights[headPitch], glm::vec3(-1.0f, 0.0f, 0.0f));
                m = glm::rotate(m, frame.weights[headYaw], glm::vec3(0.0f, 1.0f, 0.0f));
                // m = glm::translate(m, glm::vec3(0.0f, 0.0f, -20));
                auto transform = span(const_cast<float *>(glm::value_ptr(m)), 16);
                _receiver->faceLandmarks({}, frame.weights, transform, frame.frame);
            }
        } catch (exception &ex) {
            println("LiveLink::read(): {}", ex.what());
        }
    } else {
        // println("recv -> {}", nbytes);
        if (nbytes < 0) {
            perror("LiveLink::read(): recv");
        }
    }
}
