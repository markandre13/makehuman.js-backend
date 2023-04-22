#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <string>
#include <iostream>
#include <signal.h>

void ignore_sig_pipe() {
    struct sigaction act;
    memset(&act, 0, sizeof(struct sigaction));
    act.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &act, 0);
}

int create_listen_socket(const char *service)
{
    struct addrinfo hints;
    int sfd = -1;
    int r;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;
    struct addrinfo *res;
    r = getaddrinfo(0, service, &hints, &res);
    if (r != 0)
    {
        std::cerr << "getaddrinfo: " << gai_strerror(r) << std::endl;
        return -1;
    }
    for (struct addrinfo *rp = res; rp; rp = rp->ai_next)
    {
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1)
        {
            continue;
        }
        int val = 1;
        if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &val,
                       static_cast<socklen_t>(sizeof(val))) == -1)
        {
            continue;
        }
        if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
        {
            break;
        }
        close(sfd);
    }
    freeaddrinfo(res);
    if (listen(sfd, 16) == -1)
    {
        perror("listen");
        close(sfd);
        return -1;
    }
    return sfd;
}

int make_non_block(int fd)
{
    int flags, r;
    while ((flags = fcntl(fd, F_GETFL, 0)) == -1 && errno == EINTR)
        ;
    if (flags == -1)
    {
        return -1;
    }
    while ((r = fcntl(fd, F_SETFL, flags | O_NONBLOCK)) == -1 && errno == EINTR)
        ;
    if (r == -1)
    {
        return -1;
    }
    return 0;
}