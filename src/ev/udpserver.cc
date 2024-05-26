#include "udpserver.hh"

#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/errno.h>

#include <print>

using namespace std;

struct UDPServer::handler_t {
        ev_io watcher;
        UDPServer *object;
};

UDPServer::UDPServer(struct ev_loop *loop, unsigned port) : loop(loop), port(port) {
    fd = socket(AF_INET, SOCK_DGRAM, 0);

    int yes = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

    sockaddr_in name;
    name.sin_family = AF_INET;
    name.sin_addr.s_addr = htonl(INADDR_ANY);
    name.sin_port = htons(port);

    if (::bind(fd, (sockaddr *)&name, sizeof(sockaddr_in)) < 0) {
        auto error = strerror(errno);
        close(fd);
        fd = -1;
        throw runtime_error(format("failed to bind to udp socket {}: {}", port, error));
    }

    handler = new handler_t;
    handler->object = this;
    ev_io_init(&handler->watcher, libev_read_cb, fd, EV_READ);
    ev_io_start(loop, &handler->watcher);

    println("start listening on udp port {}", port);
}

UDPServer::~UDPServer() {
    println("stop listening on udp port {}", port);
    ev_io_stop(loop, &handler->watcher);
    close(fd);
    delete handler;
}

void UDPServer::libev_read_cb(struct ev_loop *loop, struct ev_io *watcher, int revents) {
    auto handler = reinterpret_cast<UDPServer::handler_t *>(watcher);
    if (EV_ERROR & revents) {
        perror("got invalid libev event");
        return;
    }
    handler->object->read();
}
