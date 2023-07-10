#include "MakeHumanHandler.hh"

#include "wslay_event.h"

using namespace std;

#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <iostream>
#include <string>

static ssize_t send_callback(wslay_event_context_ptr ctx, const uint8_t* data, size_t len, int flags, void* user_data);
static ssize_t recv_callback(wslay_event_context_ptr ctx, uint8_t* data, size_t len, int flags, void* user_data);
static void on_msg_recv_callback(wslay_event_context_ptr ctx, const struct wslay_event_on_msg_recv_arg* arg, void* user_data);

MakeHumanHandler::MakeHumanHandler(int fd) : fd_(fd) {
    struct wslay_event_callbacks callbacks = {
        recv_callback,        // called when wslay wants to read data
        send_callback,        // called when wslay wants to send data
        NULL,                 /* genmask_callback */
        NULL,                 /* on_frame_recv_start_callback */
        NULL,                 /* on_frame_recv_callback */
        NULL,                 /* on_frame_recv_end_callback */
        on_msg_recv_callback  // message received via wslay
    };
    wslay_event_context_server_init(&ctx_, &callbacks, this);
}

MakeHumanHandler::~MakeHumanHandler() {
    std::cout << "MakeHumanHandler: close" << std::endl;
    wslay_event_context_free(ctx_);
    shutdown(fd_, SHUT_WR);
    close(fd_);
}

ssize_t MakeHumanHandler::send_data(const uint8_t* data, size_t len, int flags) {
    // std::cout << "EchoWebSocketHandler::send_data(...," << len << "," << flags << ")" << std::endl;
    ssize_t r;
    int sflags = 0;
#ifdef MSG_MORE
    if (flags & WSLAY_MSG_MORE) {
        sflags |= MSG_MORE;
    }
#endif  // MSG_MORE
    while ((r = send(fd_, data, len, sflags)) == -1 && errno == EINTR)
        ;
    return r;
}

ssize_t MakeHumanHandler::recv_data(uint8_t* data, size_t len, int flags) {
    ssize_t r;
    while ((r = recv(fd_, data, len, 0)) == -1 && errno == EINTR)
        ;
    // std::cout << "EchoWebSocketHandler::recv_data(...," << len << "," << flags << ") -> " << r << std::endl;
    return r;
}

ssize_t send_callback(wslay_event_context_ptr ctx, const uint8_t* data, size_t len, int flags, void* user_data) {
    MakeHumanHandler* sv = (MakeHumanHandler*)user_data;
    ssize_t r = sv->send_data(data, len, flags);
    if (r == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            wslay_event_set_error(ctx, WSLAY_ERR_WOULDBLOCK);
        } else {
            wslay_event_set_error(ctx, WSLAY_ERR_CALLBACK_FAILURE);
        }
    }
    return r;
}

ssize_t recv_callback(wslay_event_context_ptr ctx, uint8_t* data, size_t len, int flags, void* user_data) {
    MakeHumanHandler* sv = (MakeHumanHandler*)user_data;
    ssize_t r = sv->recv_data(data, len, flags);
    if (r == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            wslay_event_set_error(ctx, WSLAY_ERR_WOULDBLOCK);
        } else {
            wslay_event_set_error(ctx, WSLAY_ERR_CALLBACK_FAILURE);
        }
    } else if (r == 0) {
        wslay_event_set_error(ctx, WSLAY_ERR_CALLBACK_FAILURE);
        r = -1;
    }
    return r;
}

static bool faceRequest = false;
bool isFaceRequested() { return faceRequest; }

static bool chordataRequest = false;
bool isChordataRequested() { return chordataRequest; }

wslay_event_context_ptr _ctx;

void sendChordata(void* data, size_t size) {
    // cout << "chordata send " << size << " octets, ctx=" << _ctx << endl;
    chordataRequest = false;
    struct wslay_event_msg msgarg = {WSLAY_BINARY_FRAME, (const uint8_t*)data, size};
    int r = wslay_event_queue_msg(_ctx, &msgarg);
    switch (r) {
        case WSLAY_ERR_NO_MORE_MSG:
            cout << "WSLAY_ERR_NO_MORE_MSG: Could not queue given message." << endl
                 << "The one of possible reason is that close control frame has been queued/sent" << endl
                 << "and no further queueing message is not allowed." << endl;

            //   return ctx->write_enabled && (ctx->close_status & WSLAY_CLOSE_QUEUED) == 0;

            if (!_ctx->write_enabled) {
                cout << "    WRITE IS NOT ENABLED" << endl;
            }
            if (!(_ctx->close_status & WSLAY_CLOSE_QUEUED)) {
                cout << "    CLOSE HAS BEEN QUEUED" << endl;
            }
            break;
        case WSLAY_ERR_INVALID_ARGUMENT:
            cout << "WSLAY_ERR_INVALID_ARGUMENT: The given message is invalid." << endl;
            break;
        case WSLAY_ERR_NOMEM:
            cout << "WSLAY_ERR_NOMEM Out of memory." << endl;
            break;
            // default:
            //     std::cout << "SEND FACE " << r << std::endl;
    }
}

void sendFace(float* float_array, int size) {
    faceRequest = false;
    struct wslay_event_msg msgarg = {WSLAY_BINARY_FRAME, (const uint8_t*)float_array, size * sizeof(float)};
    int r = wslay_event_queue_msg(_ctx, &msgarg);
    switch (r) {
        case WSLAY_ERR_NO_MORE_MSG:
            cout << "WSLAY_ERR_NO_MORE_MSG: Could not queue given message." << endl
                 << "The one of possible reason is that close control frame has been queued/sent" << endl
                 << "and no further queueing message is not allowed." << endl;

            //   return ctx->write_enabled && (ctx->close_status & WSLAY_CLOSE_QUEUED) == 0;

            if (!_ctx->write_enabled) {
                cout << "    WRITE IS NOT ENABLED" << endl;
            }
            if (!(_ctx->close_status & WSLAY_CLOSE_QUEUED)) {
                cout << "    CLOSE HAS BEEN QUEUED" << endl;
            }
            break;
        case WSLAY_ERR_INVALID_ARGUMENT:
            cout << "WSLAY_ERR_INVALID_ARGUMENT: The given message is invalid." << endl;
            break;
        case WSLAY_ERR_NOMEM:
            cout << "WSLAY_ERR_NOMEM Out of memory." << endl;
            break;
            // default:
            //     std::cout << "SEND FACE " << r << std::endl;
    }
}

void on_msg_recv_callback(wslay_event_context_ptr ctx, const struct wslay_event_on_msg_recv_arg* arg, void* user_data) {
    // cout << "on_msg_recv_callback() = '" << arg->opcode << "'" << endl;
    if (wslay_is_ctrl_frame(arg->opcode)) {
        return;
    }
    switch (arg->opcode) {
        case WSLAY_BINARY_FRAME: {
            auto msg = string((const char*)arg->msg, 0, arg->msg_length);
            // cout << "WSLAY_BINARY_FRAME '" << msg << "'" << endl;
            // auto msg = string((const char*)arg->msg, 0, arg->msg_length);
            if (msg == "GET FACE") {
                // cout << "FACE REQUESTED " << (int)arg->opcode << endl;
                _ctx = ctx;
                faceRequest = true;
                // sendFace();
            } else if (msg == "GET CHORDATA") {
                // cout << "CHORDATA REQUESTED: chordataRequest = true, ctx=" << ctx << endl;
                _ctx = ctx;
                chordataRequest = true;
            } else {
                cout << "unknown request '" << msg << "'" << endl;
            }
            // std::cout << "EchoWebSocketHandler: echo " << arg->msg_length << " bytes: \"" << msg << "\""  << std::endl;
            // struct wslay_event_msg msgarg = {arg->opcode, arg->msg, arg->msg_length};
            // wslay_event_queue_msg(ctx, &msgarg);
        } break;
        case WSLAY_CONNECTION_CLOSE: {
            cout << "CLOSE" << endl;
    //             std::cout << "MakeHumanHandler: close" << std::endl;
    // wslay_event_context_free(ctx_);
    // shutdown(fd_, SHUT_WR);
    // close(fd_);
    //         exit(0);
        } break;
        default:
            cout << "WSLAY_*_FRAME" << endl;
            auto msg = string((const char*)arg->msg, 0, arg->msg_length);
            cout << "'" << msg << "'" << endl;
    }
}
