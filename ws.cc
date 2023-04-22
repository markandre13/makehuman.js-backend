/*
 * Wslay - The WebSocket Library
 *
 * Copyright (c) 2011, 2012 Tatsuhiro Tsujikawa
 *   src: https://github.com/tatsuhiro-t/wslay/blob/master/examples/echoserv.cc
 * Copyright (c) 2023 Mark-André Hopf
 *   replaced epoll() with select() for non-Linux systems)
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
// WebSocket Echo Server
// This is suitable for Autobahn server test.
// g++ -Wall -O2 -g -o echoserv echoserv.cc -L../lib/.libs -I../lib/includes -lwslay -lnettle
// $ export LD_LIBRARY_PATH=../lib/.libs
// $ ./a.out 9000

#include "EventHandler.hh"
#include "ListenEventHandler.hh"
#include "socket.hh"

#include <cstdlib>
#include <iostream>

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " PORT" << std::endl;
        exit(EXIT_FAILURE);
    }

    ignore_sig_pipe();
    
    int sfd = create_listen_socket(argv[1]);
    if (sfd == -1)
    {
        std::cerr << "Failed to create server socket" << std::endl;
        exit(EXIT_FAILURE);
    }
    std::cout << "WebSocket echo server, listening on " << argv[1] << std::endl;
    reactor(new ListenEventHandler(sfd));
}
