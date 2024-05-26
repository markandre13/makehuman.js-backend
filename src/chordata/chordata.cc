#include "chordata.hh"
#include <sys/socket.h>
#include <print>

using namespace std;

Chordata::Chordata(struct ev_loop *loop, unsigned port, std::function<void(const char *buffer, size_t nbytes)> callback) : UDPServer(loop, port), callback(callback) {}

void Chordata::read() {
    char buffer[8192];
    ssize_t nbytes = ::recv(fd, buffer, sizeof(buffer), 0);
    if (nbytes > 0) {
        // println("chordata: received {} bytes", nbytes);
        // hexdump(buffer, nbytes);
        callback(buffer, nbytes);
    } else {
        if (nbytes < 0) {
            perror("recv");
        }
    }
}
