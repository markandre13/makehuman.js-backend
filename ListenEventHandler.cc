#include "ListenEventHandler.hh"

#include <errno.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>

#include "HttpHandshakeRecvHandler.hh"
#include "socket.hh"

ListenEventHandler::~ListenEventHandler() {
    close(fd_);
    close(cfd_);
}

int ListenEventHandler::on_read_event() {
    if (cfd_ != -1) {
        close(cfd_);
    }
    while ((cfd_ = accept(fd_, 0, 0)) == -1 && errno == EINTR)
        ;
    if (cfd_ == -1) {
        perror("accept");
    }
    std::cout << "ListenEventHandler: accepted client" << std::endl;
    return 0;
}

EventHandler *ListenEventHandler::ListenEventHandler::next() {
    if (cfd_ == -1) {
        return 0;
    }
    int val = 1;
    int fd = cfd_;
    cfd_ = -1;
    if (make_non_block(fd) == -1 || setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &val, (socklen_t)sizeof(val)) == -1) {
        close(fd);
        return 0;
    }
    // std::cout << "ListenEventHandler: start HTTP receive handler on fd " << fd << std::endl;
    return new HttpHandshakeRecvHandler(fd);
}
