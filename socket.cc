#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cerrno>

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/ip.h>

#include <wslay/wslay.h>

#include "socket.hh"

// https://github.com/tatsuhiro-t/wslay/blob/master/examples/echoserv.cc

ssize_t send_callback(wslay_event_context_ptr ctx, const uint8_t *data, size_t len, int flags, void *user_data);
ssize_t recv_callback(wslay_event_context_ptr ctx, uint8_t *data, size_t len, int flags, void *user_data);
void on_msg_recv_callback(wslay_event_context_ptr ctx, const struct wslay_event_on_msg_recv_arg *arg, void *user_data);

Socket::Socket(int port)
{
    int listenfd = socket(PF_INET, SOCK_STREAM, 0);
    if (listenfd == -1)
    {
        fprintf(stderr, "listen: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    size_t yes = -1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    struct sockaddr_in server;
    size_t size = sizeof(server);
    memset(&server, 0, size);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);
    if (bind(listenfd, (struct sockaddr *)&server, size) == -1)
    {
        fprintf(stderr, "bind: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (listen(listenfd, 16) == -1)
    {
        fprintf(stderr, "listen: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    struct wslay_event_callbacks callbacks = {
        recv_callback,
        send_callback,
        NULL, /* genmask_callback */
        NULL, /* on_frame_recv_start_callback */
        NULL, /* on_frame_recv_callback */
        NULL, /* on_frame_recv_end_callback */
        on_msg_recv_callback};

    wslay_event_context_ptr ctx_;
    wslay_event_context_server_init(&ctx_, &callbacks, this);

    // struct sockaddr_in client;
    // int connfd = accept(listenfd, (struct sockaddr *)&client, (socklen_t *)&size);
    // if (connfd == -1)
    // {
    //     fprintf(stderr, "accept: %s\n", strerror(errno));
    //     exit(EXIT_FAILURE);
    // }
    //
    // close(connfd);
}

ssize_t send_callback(
    wslay_event_context_ptr ctx,
    const uint8_t *data,
    size_t len,
    int flags,
    void *user_data)
{
    //   EchoWebSocketHandler *sv = (EchoWebSocketHandler *)user_data;
    //   ssize_t r = sv->send_data(data, len, flags);
    //   if (r == -1) {
    //     if (errno == EAGAIN || errno == EWOULDBLOCK) {
    //       wslay_event_set_error(ctx, WSLAY_ERR_WOULDBLOCK);
    //     } else {
    //       wslay_event_set_error(ctx, WSLAY_ERR_CALLBACK_FAILURE);
    //     }
    //   }
    //   return r;
}

ssize_t recv_callback(
    wslay_event_context_ptr ctx,
    uint8_t *data,
    size_t len,
    int flags,
    void *user_data)
{
    //   EchoWebSocketHandler *sv = (EchoWebSocketHandler *)user_data;
    //   ssize_t r = sv->recv_data(data, len, flags);
    //   if (r == -1) {
    //     if (errno == EAGAIN || errno == EWOULDBLOCK) {
    //       wslay_event_set_error(ctx, WSLAY_ERR_WOULDBLOCK);
    //     } else {
    //       wslay_event_set_error(ctx, WSLAY_ERR_CALLBACK_FAILURE);
    //     }
    //   } else if (r == 0) {
    //     wslay_event_set_error(ctx, WSLAY_ERR_CALLBACK_FAILURE);
    //     r = -1;
    //   }
    //   return r;
}

void on_msg_recv_callback(
    wslay_event_context_ptr ctx,
    const struct wslay_event_on_msg_recv_arg *arg,
    void *user_data)
{
    if (!wslay_is_ctrl_frame(arg->opcode))
    {
        struct wslay_event_msg msgarg = {arg->opcode, arg->msg, arg->msg_length};
        wslay_event_queue_msg(ctx, &msgarg);
    }
}
