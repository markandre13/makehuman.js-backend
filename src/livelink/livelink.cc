#include "livelink.hh"

#include <sys/socket.h>

#include <corba/hexdump.hh>
#include <print>
#include <string>

#include "livelinkframe.hh"

using namespace std;

LiveLink::LiveLink(struct ev_loop *loop, unsigned port, std::function<void(const LiveLinkFrame &)> callback) : UDPServer(loop, port), callback(callback) {}

LiveLink::~LiveLink() {}

CaptureEngine::~CaptureEngine() {}

void LiveLink::read() {
    unsigned char buffer[8192];
    ssize_t nbytes = ::recv(fd, buffer, sizeof(buffer), 0);
    if (nbytes > 0) {
        // println("livelink: received {} bytes", nbytes);
        // hexdump(buffer, nbytes);
        try {
            LiveLinkFrame frame(buffer, nbytes);
            callback(frame);
        } catch(exception &ex) {
            println("LiveLink::read(): {}", ex.what());
        }
    } else {
        // println("recv -> {}", nbytes);
        if (nbytes < 0) {
            perror("recv");
        }
    }
}
