#include "EchoWebSocketHandler.hh"

#include <cerrno>
#include <unistd.h>
#include <sys/socket.h>

static ssize_t send_callback(
    wslay_event_context_ptr ctx,
    const uint8_t *data, size_t len,
    int flags,
    void *user_data);
static ssize_t recv_callback(
    wslay_event_context_ptr ctx,
    uint8_t *data, size_t len,
    int flags,
    void *user_data);
static void on_msg_recv_callback(
    wslay_event_context_ptr ctx,
    const struct wslay_event_on_msg_recv_arg *arg,
    void *user_data);

EchoWebSocketHandler::EchoWebSocketHandler(int fd) : fd_(fd)
{
    struct wslay_event_callbacks callbacks = {
        recv_callback,
        send_callback,
        NULL, /* genmask_callback */
        NULL, /* on_frame_recv_start_callback */
        NULL, /* on_frame_recv_callback */
        NULL, /* on_frame_recv_end_callback */
        on_msg_recv_callback};
    wslay_event_context_server_init(&ctx_, &callbacks, this);
}

EchoWebSocketHandler::~EchoWebSocketHandler()
{
    wslay_event_context_free(ctx_);
    shutdown(fd_, SHUT_WR);
    close(fd_);
}

ssize_t EchoWebSocketHandler::send_data(const uint8_t *data, size_t len, int flags)
{
    ssize_t r;
    int sflags = 0;
#ifdef MSG_MORE
    if (flags & WSLAY_MSG_MORE)
    {
        sflags |= MSG_MORE;
    }
#endif // MSG_MORE
    while ((r = send(fd_, data, len, sflags)) == -1 && errno == EINTR)
        ;
    return r;
}

ssize_t EchoWebSocketHandler::recv_data(uint8_t *data, size_t len, int flags)
{
    ssize_t r;
    while ((r = recv(fd_, data, len, 0)) == -1 && errno == EINTR)
        ;
    return r;
}

ssize_t send_callback(wslay_event_context_ptr ctx, const uint8_t *data,
                      size_t len, int flags, void *user_data)
{
    EchoWebSocketHandler *sv = (EchoWebSocketHandler *)user_data;
    ssize_t r = sv->send_data(data, len, flags);
    if (r == -1)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            wslay_event_set_error(ctx, WSLAY_ERR_WOULDBLOCK);
        }
        else
        {
            wslay_event_set_error(ctx, WSLAY_ERR_CALLBACK_FAILURE);
        }
    }
    return r;
}

ssize_t recv_callback(wslay_event_context_ptr ctx, uint8_t *data, size_t len,
                      int flags, void *user_data)
{
    EchoWebSocketHandler *sv = (EchoWebSocketHandler *)user_data;
    ssize_t r = sv->recv_data(data, len, flags);
    if (r == -1)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            wslay_event_set_error(ctx, WSLAY_ERR_WOULDBLOCK);
        }
        else
        {
            wslay_event_set_error(ctx, WSLAY_ERR_CALLBACK_FAILURE);
        }
    }
    else if (r == 0)
    {
        wslay_event_set_error(ctx, WSLAY_ERR_CALLBACK_FAILURE);
        r = -1;
    }
    return r;
}

void on_msg_recv_callback(wslay_event_context_ptr ctx,
                          const struct wslay_event_on_msg_recv_arg *arg,
                          void *user_data)
{
    if (!wslay_is_ctrl_frame(arg->opcode))
    {
        struct wslay_event_msg msgarg = {arg->opcode, arg->msg, arg->msg_length};
        wslay_event_queue_msg(ctx, &msgarg);
    }
}
