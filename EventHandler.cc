
#include "EventHandler.hh"
#include <sys/select.h>

#include <set>

void reactor(EventHandler *listen_handler)
{
    std::set<EventHandler *> handlers;
    handlers.insert(listen_handler);

    while (1)
    {
        fd_set rd, wr, ex;
        int fd_max = listen_handler->fd();
        FD_ZERO(&rd);
        FD_ZERO(&wr);
        FD_ZERO(&ex);
        for (auto h = handlers.begin(); h != handlers.end(); ++h)
        {
            FD_SET((*h)->fd(), &rd);
            FD_SET((*h)->fd(), &wr);
            FD_SET((*h)->fd(), &ex);
            if ((*h)->fd() > fd_max)
            {
                fd_max = (*h)->fd();
            }
        }

        select(fd_max + 1, &rd, &wr, &ex, NULL);

        for (auto p = handlers.begin(); p != handlers.end(); ++p)
        {
redo:            
            if ((FD_ISSET((*p)->fd(), &rd) && (*p)->on_read_event() == -1) ||
                (FD_ISSET((*p)->fd(), &wr) && (*p)->on_write_event() == -1) ||
                (FD_ISSET((*p)->fd(), &ex)))
            {
                delete *p;
                handlers.erase(p);
                p = handlers.begin();
                goto redo;
            }

            EventHandler *next = (*p)->next();
            if (next) {
                handlers.insert(next);
                p = handlers.begin();
                goto redo;
            }

            if ((*p)->finish()) {
                delete *p;
                handlers.erase(p);
                p = handlers.begin();
                goto redo;
            }
        }
    }
}
