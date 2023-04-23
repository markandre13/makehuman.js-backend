#include "EchoWebSocketHandler.hh"
// #include <wslay/wslay_event.h>

enum wslay_event_close_status { WSLAY_CLOSE_RECEIVED = 1 << 0, WSLAY_CLOSE_QUEUED = 1 << 1, WSLAY_CLOSE_SENT = 1 << 2 };

struct wslay_event_context {
        /* config status, bitwise OR of enum wslay_event_config values*/
        uint32_t config;
        /* maximum message length that can be received */
        uint64_t max_recv_msg_length;
        /* 1 if initialized for server, otherwise 0 */
        uint8_t server;
        /* bitwise OR of enum wslay_event_close_status values */
        uint8_t close_status;
        /* status code in received close control frame */
        uint16_t status_code_recv;
        /* status code in sent close control frame */
        uint16_t status_code_sent;
        wslay_frame_context_ptr frame_ctx;
        /* 1 if reading is enabled, otherwise 0. Upon receiving close
           control frame this value set to 0. If any errors in read
           operation will also set this value to 0. */
        uint8_t read_enabled;
        /* 1 if writing is enabled, otherwise 0 Upon completing sending
           close control frame, this value set to 0. If any errors in write
           opration will also set this value to 0. */
        uint8_t write_enabled;
        /* imsg buffer to allow interleaved control frame between
           non-control frames. */
        //   struct wslay_event_imsg imsgs[2];
        //   /* Pointer to imsgs to indicate current used buffer. */
        //   struct wslay_event_imsg *imsg;
        //   /* payload length of frame currently being received. */
        //   uint64_t ipayloadlen;
        //   /* next byte offset of payload currently being received. */
        //   uint64_t ipayloadoff;
        //   /* error value set by user callback */
        //   int error;
        //   /* Pointer to the message currently being sent. NULL if no message
        //      is currently sent. */
        //   struct wslay_event_omsg *omsg;
        //   /* Queue for non-control frames */
        //   struct wslay_queue /*<wslay_omsg*>*/ send_queue;
        //   /* Queue for control frames */
        //   struct wslay_queue /*<wslay_omsg*>*/ send_ctrl_queue;
        //   /* Size of send_queue + size of send_ctrl_queue */
        //   size_t queued_msg_count;
        //   /* The sum of message length in send_queue */
        //   size_t queued_msg_length;
        //   /* Buffer used for fragmented messages */
        //   uint8_t obuf[4096];
        //   uint8_t *obuflimit;
        //   uint8_t *obufmark;
        //   /* payload length of frame currently being sent. */
        //   uint64_t opayloadlen;
        //   /* next byte offset of payload currently being sent. */
        //   uint64_t opayloadoff;
        //   struct wslay_event_callbacks callbacks;
        //   struct wslay_event_frame_user_data frame_user_data;
        //   void *user_data;
        //   uint8_t allowed_rsv_bits;
};

#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <iostream>
#include <string>

static ssize_t send_callback(wslay_event_context_ptr ctx, const uint8_t* data, size_t len, int flags, void* user_data);
static ssize_t recv_callback(wslay_event_context_ptr ctx, uint8_t* data, size_t len, int flags, void* user_data);
static void on_msg_recv_callback(wslay_event_context_ptr ctx, const struct wslay_event_on_msg_recv_arg* arg, void* user_data);

EchoWebSocketHandler::EchoWebSocketHandler(int fd) : fd_(fd) {
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

EchoWebSocketHandler::~EchoWebSocketHandler() {
    std::cout << "EchoWebSocketHandler: close" << std::endl;
    wslay_event_context_free(ctx_);
    shutdown(fd_, SHUT_WR);
    close(fd_);
}

ssize_t EchoWebSocketHandler::send_data(const uint8_t* data, size_t len, int flags) {
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

ssize_t EchoWebSocketHandler::recv_data(uint8_t* data, size_t len, int flags) {
    ssize_t r;
    while ((r = recv(fd_, data, len, 0)) == -1 && errno == EINTR)
        ;
    // std::cout << "EchoWebSocketHandler::recv_data(...," << len << "," << flags << ") -> " << r << std::endl;
    return r;
}

ssize_t send_callback(wslay_event_context_ptr ctx, const uint8_t* data, size_t len, int flags, void* user_data) {
    EchoWebSocketHandler* sv = (EchoWebSocketHandler*)user_data;
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
    EchoWebSocketHandler* sv = (EchoWebSocketHandler*)user_data;
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

bool faceRequest = false;
bool isFaceRequested() { return faceRequest; }

wslay_event_context_ptr _ctx;

void sendFace(float* float_array, int size) {
    faceRequest = false;
    struct wslay_event_msg msgarg = {WSLAY_BINARY_FRAME, (const uint8_t*)float_array, size * sizeof(float)};
    int r = wslay_event_queue_msg(_ctx, &msgarg);
    switch (r) {
        case WSLAY_ERR_NO_MORE_MSG:
            std::cout << "WSLAY_ERR_NO_MORE_MSG: Could not queue given message." << std::endl
                      << "The one of possible reason is that close control frame has been queued/sent" << std::endl
                      << "and no further queueing message is not allowed." << std::endl;

            //   return ctx->write_enabled && (ctx->close_status & WSLAY_CLOSE_QUEUED) == 0;

            if (!_ctx->write_enabled) {
                std::cout << "    WRITE IS NOT ENABLED" << std::endl;
            }
            if (!_ctx->close_status & WSLAY_CLOSE_QUEUED) {
                std::cout << "    CLOSE HAS BEEN QUEUED" << std::endl;
            }
            break;
        case WSLAY_ERR_INVALID_ARGUMENT:
            std::cout << "WSLAY_ERR_INVALID_ARGUMENT: The given message is invalid." << std::endl;
            break;
        case WSLAY_ERR_NOMEM:
            std::cout << "WSLAY_ERR_NOMEM Out of memory." << std::endl;
            break;
        default:
            std::cout << "SEND FACE " << r << std::endl;
    }
}

void on_msg_recv_callback(wslay_event_context_ptr ctx, const struct wslay_event_on_msg_recv_arg* arg, void* user_data) {
    // if (!wslay_is_ctrl_frame(arg->opcode))
    if (arg->opcode == WSLAY_BINARY_FRAME) {
        auto msg = std::string((const char*)arg->msg, 0, arg->msg_length);
        if (msg == "GET FACE") {
            std::cout << "FACE REQUESTED " << (int)arg->opcode << std::endl;
            _ctx = ctx;
            faceRequest = true;
            // sendFace();
        }
        // std::cout << "EchoWebSocketHandler: echo " << arg->msg_length << " bytes: \"" << msg << "\""  << std::endl;
        // struct wslay_event_msg msgarg = {arg->opcode, arg->msg, arg->msg_length};
        // wslay_event_queue_msg(ctx, &msgarg);
    }
}
