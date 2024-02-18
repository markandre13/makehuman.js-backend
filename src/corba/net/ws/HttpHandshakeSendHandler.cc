#include "HttpHandshakeSendHandler.hh"

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>

#include "CorbaHandler.hh"

HttpHandshakeSendHandler::HttpHandshakeSendHandler(int fd, const std::string &accept_key)
    : fd_(fd),
      resheaders_(
          "HTTP/1.1 101 Switching Protocols\r\n"
          "Upgrade: websocket\r\n"
          "Connection: Upgrade\r\n"
          "Sec-WebSocket-Accept: " +
          accept_key +
          "\r\n"
          "\r\n"),
      off_(0) {}

HttpHandshakeSendHandler::~HttpHandshakeSendHandler() {
    if (fd_ != -1) {
        shutdown(fd_, SHUT_WR);
        close(fd_);
    }
}

int HttpHandshakeSendHandler::on_write_event() {
    while (1) {
        size_t len = resheaders_.size() - off_;
        if (len == 0) {
            break;
        }
        ssize_t r;
        while ((r = write(fd_, resheaders_.c_str() + off_, len)) == -1 && errno == EINTR)
            ;
        if (r == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            } else {
                perror("write");
                return -1;
            }
        } else {
            off_ += r;
        }
    }
    return 0;
}

EventHandler *HttpHandshakeSendHandler::next() {
    if (finish()) {
        int fd = fd_;
        fd_ = -1;
        return new CorbaHandler(fd);
    } else {
        return 0;
    }
}
