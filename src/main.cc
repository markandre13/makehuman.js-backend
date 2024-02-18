// mature but too much: libwebsockets, libuv

#include <ev.h>
#include <wslay/wslay.h>

// #include <uv.h>
// #include <libwebsockets.h>

// a single header file is required
#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>  // for puts
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <print>
#include <string>

#include "corba/net/ws/createAcceptKey.hh"
#include "corba/net/ws/socket.hh"

#include "corba/corba.hh"
#include "corba/orb.hh"
#include "corba/protocol.hh"

using namespace std;

static void libev_accept_cb(struct ev_loop *loop, struct ev_io *watcher, int revents);
static void libev_read_cb(struct ev_loop *loop, struct ev_io *watcher, int revents);

static ssize_t wslay_send_callback(wslay_event_context_ptr ctx, const uint8_t *data, size_t len, int flags, void *user_data);
static ssize_t wslay_recv_callback(wslay_event_context_ptr ctx, uint8_t *data, size_t len, int flags, void *user_data);
static void wslay_msg_callback(wslay_event_context_ptr ctx, const struct wslay_event_on_msg_recv_arg *arg, void *user_data);

struct accept_handler_t {
    ev_io watcher;
    struct ev_loop *loop;
    CORBA::ORB *orb;
};

enum state_t { STATE_HTTP, STATE_WS };

class MyConnection;

struct client_handler_t {
        ev_io watcher;
        struct ev_loop *loop;

        // for initial HTTP negotiation
        state_t state = STATE_HTTP;
        std::string headers;

        // wslay
        wslay_event_context_ptr ctx;

        // for forwarding data from wslay to corba
        CORBA::ORB *orb;
        MyConnection *connection;

        // void libuv_read_cb(ssize_t nbytes, const uv_buf_t *buf);
};

class MyConnection : public CORBA::detail::Connection {
        std::string m_localAddress;
        uint16_t m_localPort;
        std::string m_remoteAddress;
        uint16_t m_remotePort;

    public:
        MyConnection(const std::string &localAddress, uint16_t localPort, const std::string &remoteAddress, uint16_t remotePort)
            : m_localAddress(localAddress), m_localPort(localPort), m_remoteAddress(remoteAddress), m_remotePort(remotePort) {}

        std::string localAddress() const override { return m_localAddress; }
        uint16_t localPort() const override { return m_localPort; }
        std::string remoteAddress() const override { return m_remoteAddress; }
        uint16_t remotePort() const override { return m_remotePort; }

        void close() override;
        void send(void *buffer, size_t nbyte) override;

        client_handler_t * handler;
};

void MyConnection::close() {};

void MyConnection::send(void *buffer, size_t nbyte) {
    println("MyConnection::send(..., {})", nbyte);
    struct wslay_event_msg msgarg = {WSLAY_BINARY_FRAME, (const uint8_t*)buffer, nbyte};
    int r0 = wslay_event_queue_msg(this->handler->ctx, &msgarg);
    println("wslay_event_queue_msg() -> {}", r0);
    int r1 = wslay_event_send(this->handler->ctx);
    println("wslay_event_send() -> {}", r1);
}

class Backend_skel : public CORBA::Skeleton {
    public:
        Backend_skel(CORBA::ORB *orb) : Skeleton(orb) {}
        const char *repository_id() const override { return "IDL:Server:1.0"; }
};

class Backend_impl : public Backend_skel {
    public:
        Backend_impl(CORBA::ORB *orb) : Backend_skel(orb) {}

    protected:
        CORBA::task<> _call(const std::string_view &operation, CORBA::GIOPDecoder &decoder, CORBA::GIOPEncoder &encoder);
};

CORBA::task<> Backend_impl::_call(const std::string_view &operation, CORBA::GIOPDecoder &decoder, CORBA::GIOPEncoder &encoder) { 
    println("Backend_impl::_call(\"{}\", decoder, encoder)", operation);
    encoder.ushort(4711);
    co_return;
}

int main(void) {

    auto orb = new CORBA::ORB();
    // auto protocol = new CORBA::WsProtocol(orb);
    // orb->registerProtocol(protocol);
    // new Backend_impl(orb);
    orb->bind("Backend", make_shared<Backend_impl>(orb));
    // protocol->listen("0.0.0.0", 9001);

    printf("running\n");
    // use the default event loop unless you have special needs
    struct ev_loop *loop = EV_DEFAULT;

    int fd = create_listen_socket("localhost", 9001);
    accept_handler_t accept_watcher;
    accept_watcher.loop = loop;
    accept_watcher.orb = orb;
    ev_io_init(&accept_watcher.watcher, libev_accept_cb, fd, EV_READ);
    ev_io_start(loop, &accept_watcher.watcher);

    // initialise a timer watcher, then start it
    // simple non-repeating 5.5 second timeout
    // ev_timer_init(&timeout_watcher, timeout_cb, 5.5, 0.);
    // ev_timer_start(loop, &timeout_watcher);

    ev_run(loop, 0);

    return 0;
}

void libev_accept_cb(struct ev_loop *loop, struct ev_io *watcher, int revents) {
    auto handler = reinterpret_cast<accept_handler_t*>(watcher);
    puts("got client");
    if (EV_ERROR & revents) {
        perror("got invalid event");
        return;
    }

    int fd = accept(watcher->fd, 0, 0);
    int val = 1;
    if (make_non_block(fd) == -1 || setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &val, (socklen_t)sizeof(val)) == -1) {
        puts("failed to setup");
        close(fd);
        return;
    }
    auto client_handler = new client_handler_t();
    client_handler->loop = loop;
    client_handler->orb = handler->orb;
    client_handler->connection = new MyConnection("localhost", 9001, "frontend", 2);
    client_handler->connection->handler = client_handler;
    ev_io_init(&client_handler->watcher, libev_read_cb, fd, EV_READ);
    ev_io_start(loop, &client_handler->watcher);
}

void libev_read_cb(struct ev_loop *loop, struct ev_io *watcher, int revents) {
    if (EV_ERROR & revents) {
        perror("libev_read_cb(): got invalid event");
        return;
    }
    auto handler = reinterpret_cast<client_handler_t *>(watcher);
    switch (handler->state) {
        case STATE_HTTP: {
            char buffer[8192];
            ssize_t nbytes = recv(watcher->fd, buffer, 8192, 0);
            if (nbytes < 0) {
                perror("read error");
                return;
            }
            if (nbytes == 0) {
                if (errno == EINVAL) {
                    println("peer closed");
                } else {
                    perror("peer might be closing");
                }
                ev_io_stop(loop, watcher);
                if (close(watcher->fd) != 0) {
                    perror("close");
                }
                delete handler;
                return;
            }

            println("rcvd {} bytes", nbytes);
            handler->headers.append(buffer, nbytes);
            if (handler->headers.size() > 8192) {
                std::cerr << "Too large http header" << std::endl;
            }
            if (handler->headers.find("\r\n\r\n") != std::string::npos) {
                std::string::size_type keyhdstart;
                if (handler->headers.find("Upgrade: websocket\r\n") == std::string::npos ||
                    handler->headers.find("Connection: Upgrade\r\n") == std::string::npos ||
                    (keyhdstart = handler->headers.find("Sec-WebSocket-Key: ")) == std::string::npos) {
                    std::cerr << "http_upgrade: missing required headers" << std::endl;
                    // abort
                    return;
                }
                keyhdstart += 19;
                std::string::size_type keyhdend = handler->headers.find("\r\n", keyhdstart);
                string client_key = handler->headers.substr(keyhdstart, keyhdend - keyhdstart);
                string accept_key = create_acceptkey(client_key);
                println("got HTTP request, switching to websocket");

                handler->headers.clear();
                handler->state = STATE_WS;

                string reply =
                    "HTTP/1.1 101 Switching Protocols\r\n"
                    "Upgrade: websocket\r\n"
                    "Connection: Upgrade\r\n"
                    "Sec-WebSocket-Accept: " +
                    accept_key +
                    "\r\n"
                    "\r\n";
                send(watcher->fd, reply.data(), reply.size(), 0);

                struct wslay_event_callbacks callbacks = {
                    wslay_recv_callback,  // called when wslay wants to read data
                    wslay_send_callback,  // called when wslay wants to send data
                    NULL,                 /* genmask_callback */
                    NULL,                 /* on_frame_recv_start_callback */
                    NULL,                 /* on_frame_recv_callback */
                    NULL,                 /* on_frame_recv_end_callback */
                    wslay_msg_callback    // message received via wslay
                };
                wslay_event_context_server_init(&handler->ctx, &callbacks, handler);
            }

            printf("message:%s\n", buffer);
        } break;
        case STATE_WS:
            printf("WS\n");
            wslay_event_recv(handler->ctx);
            break;
    }
}

ssize_t wslay_send_callback(wslay_event_context_ptr ctx, const uint8_t *data, size_t len, int flags, void *user_data) {
    auto handler = reinterpret_cast<client_handler_t *>(user_data);
    println("wslay_send_callback");
    auto r = send(handler->watcher.fd, (void*)data, len, 0);
    println("send() -> {}", r);
    return r;
}
ssize_t wslay_recv_callback(wslay_event_context_ptr ctx, uint8_t *data, size_t len, int flags, void *user_data) {
    auto handler = reinterpret_cast<client_handler_t *>(user_data);
    ssize_t nbytes = recv(handler->watcher.fd, data, len, 0);
    println("wslay_recv_callback -> {}", nbytes);
    if (nbytes < 0) {
        if (errno != EAGAIN) {
            println("errno = {}", errno);
            perror("recv error");
        }
    }
    if (nbytes == 0) {
        if (errno == EAGAIN) {
            println("peer closed");
        } else {
            perror("peer might be closing");
        }
        ev_io_stop(handler->loop, &handler->watcher);
        if (close(handler->watcher.fd) != 0) {
            perror("close");
        }
        delete handler;
    }

    //
    // ssize_t r;
    // while ((r = recv(handler->watcher.fd, data, len, 0)) == -1 && errno == EINTR)
    //     ;
    return nbytes;
}
void wslay_msg_callback(wslay_event_context_ptr ctx, const struct wslay_event_on_msg_recv_arg *arg, void *user_data) {
    println("wslay_msg_callback");
    auto handler = reinterpret_cast<client_handler_t *>(user_data);
    handler->orb->_socketRcvd(handler->connection, arg->msg, arg->msg_length).no_wait();
}