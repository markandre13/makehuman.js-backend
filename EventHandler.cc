
#include "EventHandler.hh"
#include <sys/select.h>

#include <set>
#include <iostream>

void reactor(EventHandler *listen_handler)
{
    std::set<EventHandler *> handlers;
    handlers.insert(listen_handler);

    while (true)
    {
        fd_set rd, wr, ex;
        int fd_max = listen_handler->fd();
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
        select(fd_max + 1, &rd, &wr, &ex, NULL);

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
