#include "ListenEventHandler.hh"
#include "HttpHandshakeRecvHandler.hh"
#include "socket.hh"
#include <unistd.h>
#include <errno.h>

#include <sys/socket.h>
#include <netinet/tcp.h>

ListenEventHandler::~ListenEventHandler()
{
    close(fd_);
    close(cfd_);
}

int ListenEventHandler::on_read_event()
{
    if (cfd_ != -1)
    {
        close(cfd_);
    }
    while ((cfd_ = accept(fd_, 0, 0)) == -1 && errno == EINTR)
        ;
    if (cfd_ == -1)
    {
        perror("accept");
    }
    return 0;
}

EventHandler *ListenEventHandler::ListenEventHandler::next()
{
    if (cfd_ != -1)
    {
        int val = 1;
        int fd = cfd_;
        cfd_ = -1;
        if (make_non_block(fd) == -1 ||
            setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &val, (socklen_t)sizeof(val)) == -1)
        {
            close(fd);
            return 0;
        }
        return new HttpHandshakeRecvHandler(fd);
    }
    else
    {
        return 0;
    }
}
