#include "HttpHandshakeRecvHandler.hh"
#include "HttpHandshakeSendHandler.hh"
#include "createAcceptKey.hh"
#include <unistd.h>
#include <iostream>

HttpHandshakeRecvHandler::~HttpHandshakeRecvHandler()
{
    if (fd_ != -1)
    {
        close(fd_);
    }
}
int HttpHandshakeRecvHandler::on_read_event()
{
    char buf[4096];
    ssize_t r;
    std::string client_key;
    while (true)
    {
        while ((r = read(fd_, buf, sizeof(buf))) == -1 && errno == EINTR)
            ;
        if (r == -1)
        {
            if (errno == EWOULDBLOCK || errno == EAGAIN)
            {
                break;
            }
            else
            {
                perror("read");
                return -1;
            }
        }
        else if (r == 0)
        {
            std::cerr << "http_upgrade: Got EOF" << std::endl;
            return -1;
        }
        else
        {
            headers_.append(buf, buf + r);
            if (headers_.size() > 8192)
            {
                std::cerr << "Too large http header" << std::endl;
                return -1;
            }
        }
    }
    if (headers_.find("\r\n\r\n") != std::string::npos)
    {
        std::string::size_type keyhdstart;
        if (headers_.find("Upgrade: websocket\r\n") == std::string::npos ||
            headers_.find("Connection: Upgrade\r\n") == std::string::npos ||
            (keyhdstart = headers_.find("Sec-WebSocket-Key: ")) ==
                std::string::npos)
        {
            std::cerr << "http_upgrade: missing required headers" << std::endl;
            return -1;
        }
        keyhdstart += 19;
        std::string::size_type keyhdend = headers_.find("\r\n", keyhdstart);
        client_key = headers_.substr(keyhdstart, keyhdend - keyhdstart);
        accept_key_ = create_acceptkey(client_key);

        // std::cout << "HttpHandshakeRecvHandler: got HTTP request" << std::endl;
    }
    return 0;
}

EventHandler *HttpHandshakeRecvHandler::next()
{
    if (finish())
    {
        int fd = fd_;
        fd_ = -1;
        // std::cout << "HttpHandshakeRecvHandler: start HTTP send handler on fd " << fd << std::endl;
        return new HttpHandshakeSendHandler(fd, accept_key_);
    }
    else
    {
        return 0;
    }
}
