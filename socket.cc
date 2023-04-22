#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <string>
#include <vector>

#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include <nettle/base64.h>
#include <nettle/sha1.h>
#include <wslay/wslay.h>

#include "socket.hh"

using std::string;
using std::vector;

int main() {
    auto socket = new Socket(9001);
    return 0;
}

// https://github.com/tatsuhiro-t/wslay/blob/master/examples/echoserv.cc

vector<string> split(string s, string delimiter);

string sha1(const string &src)
{
    sha1_ctx ctx;
    sha1_init(&ctx);
    sha1_update(&ctx, src.size(), reinterpret_cast<const uint8_t *>(src.c_str()));
    uint8_t temp[SHA1_DIGEST_SIZE];
    sha1_digest(&ctx, SHA1_DIGEST_SIZE, temp);
    string res(&temp[0], &temp[SHA1_DIGEST_SIZE]);
    return res;
}

string base64(const string &src)
{
    base64_encode_ctx ctx;
    base64_encode_init(&ctx);
    int dstlen = BASE64_ENCODE_RAW_LENGTH(src.size());
    char *dst = new char[dstlen];
    base64_encode_raw(dst, src.size(), reinterpret_cast<const uint8_t *>(src.c_str()));
    string res(&dst[0], &dst[dstlen]);
    delete[] dst;
    return res;
}

ssize_t send_callback(wslay_event_context_ptr ctx, const uint8_t *data, size_t len, int flags, void *user_data);
ssize_t recv_callback(wslay_event_context_ptr ctx, uint8_t *data, size_t len, int flags, void *user_data);
void on_msg_recv_callback(wslay_event_context_ptr ctx, const struct wslay_event_on_msg_recv_arg *arg, void *user_data);

int make_non_block(int fd)
{
    int flags, r;
    while ((flags = fcntl(fd, F_GETFL, 0)) == -1 && errno == EINTR)
        ;
    if (flags == -1)
    {
        return -1;
    }
    while ((r = fcntl(fd, F_SETFL, flags | O_NONBLOCK)) == -1 && errno == EINTR)
        ;
    if (r == -1)
    {
        return -1;
    }
    return 0;
}

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

    struct sockaddr_in client;
    int connfd = accept(listenfd, (struct sockaddr *)&client, (socklen_t *)&size);
    if (connfd == -1)
    {
        fprintf(stderr, "accept: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // int val = 1;
    // if (make_non_block(connfd) == -1 ||
    //     setsockopt(connfd, IPPROTO_TCP, TCP_NODELAY, &val,
    //                (socklen_t)sizeof(val)) == -1)
    // {
    //     close(connfd);
    //     return;
    // }

    //
    // HTTP STEP
    //

    char buf[4096];
    ssize_t r;
    r = read(connfd, buf, sizeof(buf));

    printf("RECEIVED\n");
    fwrite(buf, r, 1, stdout);

    string b(buf, 0, r);
    auto lines = split(b, "\r\n");
    auto connectionUpgrade = false;
    auto upgradeWebSocket = false;
    string secWebSocketKey;
    // FIXME: headers are usually case insensitive...
    for (auto i = lines.begin(); i < lines.end(); ++i)
    {
        // printf("%s\n", i->c_str());
        if (*i == "Connection: Upgrade")
        {
            connectionUpgrade = true;
            continue;
        }
        if (*i == "Upgrade: websocket")
        {
            upgradeWebSocket = true;
            continue;
        }
        if (i->size() > 19 && i->compare(0, 19, "Sec-WebSocket-Key: "))
        {
            secWebSocketKey = i->substr(20);
        }
    }
    if (!connectionUpgrade || !upgradeWebSocket || secWebSocketKey.empty())
    {
        fprintf(stderr, "not a websocket\n");
        exit(EXIT_FAILURE);
    }
    printf("switch to websocket\n");

    auto accept_key = base64(sha1(secWebSocketKey + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"));

    string reply(
        "HTTP/1.1 101 Switching Protocols\r\n"
        "Connection: Upgrade\r\n"
        "Upgrade: websocket\r\n"
        "Sec-WebSocket-Accept: " +
        accept_key +
        "\r\n"
        "\r\n");
    printf("SEND:\n%s", reply.c_str());
    write(connfd, reply.c_str(), reply.size());

    struct wslay_event_callbacks callbacks = {
        recv_callback,
        send_callback,
        NULL, /* genmask_callback */
        NULL, /* on_frame_recv_start_callback */
        NULL, /* on_frame_recv_callback */
        NULL, /* on_frame_recv_end_callback */
        on_msg_recv_callback};

    this->clientfd = connfd;
    wslay_event_context_ptr ctx_;
    wslay_event_context_server_init(&ctx_, &callbacks, this);

    printf("start websocket protocol on socket %i\n", connfd);
    while(true) {
        sleep(30);
        wslay_event_recv(ctx_);
    }

    // close(connfd);
}

ssize_t send_callback(
    wslay_event_context_ptr ctx,
    const uint8_t *data,
    size_t len,
    int flags,
    void *user_data)
{
    printf("send_callback\n");
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
    auto sv = (Socket *)user_data;
    printf("recv_callback: read from %i\n", sv->clientfd);
    ssize_t r = read(sv->clientfd, data, len);
    // ssize_t r = recv(sv->clientfd, data, len, 0);
    printf("recv_callback: got %zd\n", r);
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
    else
    {
        printf("recv_callback: received %zd bytes\n", r);
    }
    return r;
}

void on_msg_recv_callback(
    wslay_event_context_ptr ctx,
    const struct wslay_event_on_msg_recv_arg *arg,
    void *user_data)
{
    printf("on_msg_recv_callback\n");
    if (!wslay_is_ctrl_frame(arg->opcode))
    {
        struct wslay_event_msg msgarg = {arg->opcode, arg->msg, arg->msg_length};
        wslay_event_queue_msg(ctx, &msgarg);
    }
}

vector<string> split(string s, string delimiter)
{
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    string token;
    vector<std::string> res;

    while ((pos_end = s.find(delimiter, pos_start)) != string::npos)
    {
        token = s.substr(pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back(token);
    }

    res.push_back(s.substr(pos_start));
    return res;
}
