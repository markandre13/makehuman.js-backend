#include "../ws.hh"

#include "../../orb.hh"
#include "createAcceptKey.hh"
// #include "wslay_event.h"

#include <stdint.h>
#include <wslay/wslay.h>

// #include "EventHandler.hh"

// #include <sys/select.h>
#include <cstdlib>
#include <iostream>
#include <set>

// #include "ListenEventHandler.hh"
// #include "socket.hh"
#include <uv.h>

using namespace std;

// libuv
static uv_loop_t *loop;
static void libuv_accept_cb(uv_stream_t *server, int status);
static void libuv_read_cb(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf);
static void libuv_alloc_cb(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);

// wslay
static ssize_t wslay_send_callback(wslay_event_context_ptr ctx, const uint8_t *data, size_t len, int flags, void *user_data);
static ssize_t wslay_recv_callback(wslay_event_context_ptr ctx, uint8_t *data, size_t len, int flags, void *user_data);
static void on_msg_recv_callback(wslay_event_context_ptr ctx, const struct wslay_event_on_msg_recv_arg *arg, void *user_data);

// corba
CORBA::ORB *orb;
CORBA::WsProtocol *protocol;

CORBA::WsProtocol::WsProtocol(CORBA::ORB *anORB) { ::orb = anORB; }

struct corba_handler_t {
        uv_tcp_t client;

        // for initial HTTP negotiation
        int state = 0;
        string headers;
        wslay_event_context_ptr ctx;

        // for forwarding data from libuv to wslay
        ssize_t rcvd_nbytes;
        const char *rcvd_buf = nullptr;

        // for forwarding data from wslay to corba
        CORBA::ORB *orb;
        CORBA::WsConnection *connection;

        void libuv_read_cb(ssize_t nbytes, const uv_buf_t *buf);
};

//
// run the libuv loop with a tcp server
//
void CORBA::WsProtocol::listen(const std::string &hostname, uint16_t port) {
    loop = uv_default_loop();

    uv_tcp_t server;
    uv_tcp_init(loop, &server);
    struct sockaddr_in addr;
    uv_ip4_addr(hostname.c_str(), port, &addr);
    uv_tcp_bind(&server, (const struct sockaddr *)&addr, 0);
    int r = uv_listen((uv_stream_t *)&server, 10, libuv_accept_cb);
    if (r) {
        fprintf(stderr, "Listen error %s\n", uv_strerror(r));
        return;
    }
    println("waiting for clients");
    uv_run(loop, UV_RUN_DEFAULT);
}

void libuv_accept_cb(uv_stream_t *server, int status) {
    if (status < 0) {
        fprintf(stderr, "New connection error %s\n", uv_strerror(status));
        // error!
        return;
    }

    corba_handler_t *handler = new corba_handler_t();
    uv_tcp_init(loop, &handler->client);
    if (uv_accept(server, (uv_stream_t *)handler) == 0) {
        println("new client");

        struct sockaddr_storage addr;
        int len = sizeof(addr);
        uv_tcp_getpeername(&handler->client, (struct sockaddr *)&addr, &len);
        if (addr.ss_family == AF_INET) {
            auto saddr = (struct sockaddr_in *)&addr;
            println("peer {}:{}", inet_ntoa(saddr->sin_addr), saddr->sin_port);
        }
        uv_tcp_getsockname(&handler->client, (struct sockaddr *)&addr, &len);
        if (addr.ss_family == AF_INET) {
            auto saddr = (struct sockaddr_in *)&addr;
            println("sock {}:{}", inet_ntoa(saddr->sin_addr), saddr->sin_port);
        }

        handler->orb = orb;
        handler->connection = new CORBA::WsConnection("backend", 1, "frontend", 2);
        handler->connection->handler = handler;
        orb->addConnection(handler->connection);

        uv_read_start((uv_stream_t *)handler, libuv_alloc_cb, libuv_read_cb);
    }
}

struct write_req_t {
        uv_write_t req;
        uv_buf_t buf;
};

void free_write_req(uv_write_t *req) {
    write_req_t *wr = (write_req_t *)req;
    free(wr->buf.base);
    free(wr);
}

void write_cb(uv_write_t *req, int status) {
    println("libuv write request done with status {}", status);
    if (status) {
        fprintf(stderr, "Write error %s\n", uv_strerror(status));
    }
    free_write_req(req);
}

void libuv_read_cb(uv_stream_t *client, ssize_t nbytes, const uv_buf_t *buf) { reinterpret_cast<corba_handler_t *>(client)->libuv_read_cb(nbytes, buf); }

void corba_handler_t::libuv_read_cb(ssize_t nbytes, const uv_buf_t *buf) {
    switch (state) {
        case 0: {
            if (nbytes < 0) {
                if (nbytes != UV_EOF) {
                    fprintf(stderr, "Read error %s\n", uv_err_name(nbytes));
                }
                uv_close((uv_handle_t *)&client, NULL);
                return;
            }
            if (nbytes > 0) {
                println("rcvd {} bytes", nbytes);
                headers.append(buf->base, nbytes);
                if (headers.size() > 8192) {
                    std::cerr << "Too large http header" << std::endl;
                }
            }
            free(buf->base);
            if (headers.find("\r\n\r\n") != std::string::npos) {
                std::string::size_type keyhdstart;
                if (headers.find("Upgrade: websocket\r\n") == std::string::npos || headers.find("Connection: Upgrade\r\n") == std::string::npos ||
                    (keyhdstart = headers.find("Sec-WebSocket-Key: ")) == std::string::npos) {
                    std::cerr << "http_upgrade: missing required headers" << std::endl;
                    // abort
                    return;
                }
                keyhdstart += 19;
                std::string::size_type keyhdend = headers.find("\r\n", keyhdstart);
                string client_key = headers.substr(keyhdstart, keyhdend - keyhdstart);
                string accept_key = create_acceptkey(client_key);
                println("got HTTP request, switching to websocket");

                headers.clear();
                state = 1;

                string reply =
                    "HTTP/1.1 101 Switching Protocols\r\n"
                    "Upgrade: websocket\r\n"
                    "Connection: Upgrade\r\n"
                    "Sec-WebSocket-Accept: " +
                    accept_key +
                    "\r\n"
                    "\r\n";
                auto sndbuf = (char *)malloc(nbytes);
                memcpy(sndbuf, reply.data(), reply.size());

                write_req_t *req = (write_req_t *)malloc(sizeof(write_req_t));
                req->buf = uv_buf_init(sndbuf, reply.size());
                uv_write((uv_write_t *)req, (uv_stream_t *)&client, &req->buf, 1, write_cb);
            }
        } break;
        case 1: {
            println("got HTTP 2nd request");
            struct wslay_event_callbacks callbacks = {
                wslay_recv_callback,        // called when wslay wants to read data
                wslay_send_callback,        // called when wslay wants to send data
                NULL,                 /* genmask_callback */
                NULL,                 /* on_frame_recv_start_callback */
                NULL,                 /* on_frame_recv_callback */
                NULL,                 /* on_frame_recv_end_callback */
                on_msg_recv_callback  // message received via wslay
            };
            wslay_event_context_server_init(&ctx, &callbacks, this);
            state = 2;
        }
        case 2: {
            println("libuv received {} bytes", nbytes);
            rcvd_nbytes = nbytes;
            rcvd_buf = buf->base;
            wslay_event_recv(ctx);  // tell wslay that we have some data
        } break;
    }
}

void libuv_alloc_cb(uv_handle_t *handle, size_t nbytes, uv_buf_t *buf) {
    buf->base = (char *)malloc(nbytes);
    buf->len = nbytes;
}

ssize_t wslay_send_callback(wslay_event_context_ptr ctx, const uint8_t *buffer, size_t nbytes, int flags, void *user_data) {
    println("wslay_send_callback() with {} bytes", nbytes);
    auto handler = (corba_handler_t *)user_data;

    write_req_t *req = (write_req_t *)malloc(sizeof(write_req_t));
    auto sndbuf = (char *)malloc(nbytes);
    memcpy(sndbuf, buffer, nbytes);

    req->buf = uv_buf_init(sndbuf, nbytes);
    uv_write((uv_write_t *)req, (uv_stream_t *)&handler->client, &req->buf, 1, write_cb);

    return 0;
    // CorbaHandler* sv = (CorbaHandler*)user_data;
    // ssize_t r = sv->send_data(data, len, flags);
    // if (r == -1) {
    //     if (errno == EAGAIN || errno == EWOULDBLOCK) {
    //         wslay_event_set_error(ctx, WSLAY_ERR_WOULDBLOCK);
    //     } else {
    //         wslay_event_set_error(ctx, WSLAY_ERR_CALLBACK_FAILURE);
    //     }
    // }
    // return r;
}

ssize_t wslay_recv_callback(wslay_event_context_ptr ctx, uint8_t *data, size_t len, int flags, void *user_data) {
    auto handler = (corba_handler_t *)user_data;
    if (handler->rcvd_buf == nullptr) {
        println("wslay_recv_callback(), no more data");
        wslay_event_set_error(ctx, WSLAY_ERR_WOULDBLOCK);
        return -1;
    }
    println("wslay recv_callback(), fill buffer of {} bytes with {} bytes", len, handler->rcvd_nbytes);
    memcpy(data, handler->rcvd_buf, handler->rcvd_nbytes);
    free((void *)handler->rcvd_buf);
    handler->rcvd_buf = nullptr;
    return handler->rcvd_nbytes;
}

void on_msg_recv_callback(wslay_event_context_ptr ctx, const struct wslay_event_on_msg_recv_arg *arg, void *user_data) {
    auto handler = (corba_handler_t *)user_data;
    cout << "wslay on_msg_recv_callback() = " << (int)arg->opcode << endl;
    if (wslay_is_ctrl_frame(arg->opcode)) {
        return;
    }
    switch (arg->opcode) {
        case WSLAY_BINARY_FRAME: {
            cout << "got binary frame with " << arg->msg_length << " bytes" << endl;
            handler->orb->_socketRcvd(handler->connection, arg->msg, arg->msg_length).no_wait();
            //         // connection->recv(arg->msg, arg->msg_length);
            //         // _ctx = ctx;
            //         // CORBA::ORB::socketRcvd(arg->msg, arg->msg_length);
            //         // auto msg = string((const char*)arg->msg, 0, arg->msg_length);
            //         // // cout << "WSLAY_BINARY_FRAME '" << msg << "'" << endl;
            //         // // auto msg = string((const char*)arg->msg, 0, arg->msg_length);
            //         // if (msg == "GET FACE") {
            //         //     // cout << "FACE REQUESTED " << (int)arg->opcode << endl;
            //         //     _ctx = ctx;
            //         //     faceRequest = true;
            //         //     // sendFace();
            //         // } else if (msg == "GET CHORDATA") {
            //         //     // cout << "CHORDATA REQUESTED: chordataRequest = true, ctx=" << ctx << endl;
            //         //     _ctx = ctx;
            //         //     chordataRequest = true;
            //         // } else {
            //         //     cout << "unknown request '" << msg << "'" << endl;
            //         // }
            //         // std::cout << "EchoWebSocketHandler: echo " << arg->msg_length << " bytes: \"" << msg << "\""  << std::endl;
            //         // struct wslay_event_msg msgarg = {arg->opcode, arg->msg, arg->msg_length};
            //         // wslay_event_queue_msg(ctx, &msgarg);
        } break;
        case WSLAY_CONNECTION_CLOSE: {
            cout << "CLOSE" << endl;
            //             std::cout << "CorbaHandler: close" << std::endl;
            // wslay_event_context_free(ctx_);
            // shutdown(fd_, SHUT_WR);
            // close(fd_);
            //         exit(0);
        } break;
        default:
            cout << "WSLAY_*_FRAME" << endl;
            auto msg = string((const char *)arg->msg, 0, arg->msg_length);
            cout << "'" << msg << "'" << endl;
    }
}

namespace CORBA {

WsConnection *WsProtocol::connect(const ORB *orb, const std::string &hostname, uint16_t port) {
    println("WsConnection::connect(\"{}\", {})", hostname, port);
    auto conn = new WsConnection(m_localAddress, m_localPort, hostname, port);
    printf("WsConnection::connect() -> %p %s:%u -> %s:%u requestId=%u\n", static_cast<void *>(conn), conn->localAddress().c_str(), conn->localPort(),
           conn->remoteAddress().c_str(), conn->remotePort(), conn->requestId);
    return conn;
}

CORBA::task<void> WsProtocol::close() { co_return; }

void WsConnection::close() {}

void WsConnection::send(void *buffer, size_t nbytes) {
    println("WsConnection::send(...) {} bytes from {}:{} to {}:{}", nbytes, m_localAddress, m_localPort, m_remoteAddress, m_remotePort);

    auto sndbuf = (char *)malloc(nbytes);
    memcpy(sndbuf, buffer, nbytes);

    println("call wslay_event_queue_msg(...)");
    // struct wslay_event_msg msgarg = {WSLAY_BINARY_FRAME, (const uint8_t *)sndbuf, nbytes};
    auto m = new wslay_event_msg;
    m->opcode = WSLAY_BINARY_FRAME;
    m->msg = (const uint8_t *)sndbuf;
    m->msg_length = nbytes;
    int r = wslay_event_queue_msg(handler->ctx, m);
    switch (r) {
        case 0: {
            // send queued messages
            println("called wslay_event_queue_msg(ctx, msg) -> {}", r);
            int x = wslay_event_send(handler->ctx);
            println("call wslay_event_send(ctx) -> {}", x);
        } break;
        case WSLAY_ERR_NO_MORE_MSG:
            cout << "WSLAY_ERR_NO_MORE_MSG: Could not queue given message." << endl
                 << "The one of possible reason is that close control frame has been queued/sent" << endl
                 << "and no further queueing message is not allowed." << endl;

            //   return ctx->write_enabled && (ctx->close_status & WSLAY_CLOSE_QUEUED) == 0;

            // if (!handler->ctx->write_enabled) {
            //     cout << "    WRITE IS NOT ENABLED" << endl;
            // }
            // if (!(handler->ctx->close_status & WSLAY_CLOSE_QUEUED)) {
            //     cout << "    CLOSE HAS BEEN QUEUED" << endl;
            // }
            break;
        case WSLAY_ERR_INVALID_ARGUMENT:
            cout << "WSLAY_ERR_INVALID_ARGUMENT: The given message is invalid." << endl;
            break;
        case WSLAY_ERR_NOMEM:
            cout << "WSLAY_ERR_NOMEM Out of memory." << endl;
            break;
        default:
            cout << "failed to queue wslay message" << endl;
            //     std::cout << "SEND FACE " << r << std::endl;
    }
}

}  // namespace CORBA
