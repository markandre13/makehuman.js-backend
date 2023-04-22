#include "EventHandler.hh"
#include "ListenEventHandler.hh"
#include "socket.hh"

#include <cstdlib>
#include <set>
#include <iostream>
#include <sys/select.h>

std::set<EventHandler *> handlers;

void wsInit()
{
    ignore_sig_pipe();

    int sfd = create_listen_socket("9001");
    if (sfd == -1)
    {
        std::cerr << "Failed to create server socket" << std::endl;
        exit(EXIT_FAILURE);
    }

    handlers.insert(new ListenEventHandler(sfd));
}

void wsHandle()
{
    while (true)
    {
        fd_set rd, wr, ex;
        int fd_max = -1;
        FD_ZERO(&rd);
        FD_ZERO(&wr);
        FD_ZERO(&ex);
        for (auto h = handlers.begin(); h != handlers.end(); ++h)
        {
            if ((*h)->want_read())
            {
                FD_SET((*h)->fd(), &rd);
            }
            if ((*h)->want_write())
            {
                FD_SET((*h)->fd(), &wr);
            }
            FD_SET((*h)->fd(), &ex);
            if ((*h)->fd() > fd_max)
            {
                fd_max = (*h)->fd();
            }
        }

        // std::cout << "waiting..." << std::endl;
        timeval t;
        t.tv_sec = 0;
        t.tv_usec = 0;
        if (select(fd_max + 1, &rd, &wr, &ex, &t) <= 0)
        {
            break;
        }

        auto p = handlers.begin();
        while (p != handlers.end())
        {
            auto fail = false;
            if (FD_ISSET((*p)->fd(), &rd))
            {
                // std::cout << (*p)->name() << " can read data on socket " << (*p)->fd() << std::endl;
                if ((*p)->on_read_event() == -1)
                {
                    std::cout << "    read failure" << std::endl;
                    fail = true;
                }
            }
            if (FD_ISSET((*p)->fd(), &wr))
            {
                // std::cout << "  can write data on socket " << (*p)->fd() << std::endl;
                if ((*p)->on_write_event() == -1)
                {
                    std::cout << "    write failure" << std::endl;
                    fail = true;
                }
            }
            if (FD_ISSET((*p)->fd(), &ex))
            {
                // std::cout << (*p)->name() << " exception on socket " << (*p)->fd() << std::endl;
                fail = true;
            }

            if (fail)
            {
                delete *p;
                p = handlers.erase(p);
                continue;
            }

            EventHandler *next = (*p)->next();
            if (next)
            {
                // std::cout << (*p)->name() << " span next handler on socket " << (*p)->fd() << std::endl;
                handlers.insert(next);
            }

            if ((*p)->finish())
            {
                // std::cout << (*p)->name() << " close handler for socket " << (*p)->fd() << std::endl;
                delete *p;
                p = handlers.erase(p);
                continue;
            }
            ++p;
        }
    }
}