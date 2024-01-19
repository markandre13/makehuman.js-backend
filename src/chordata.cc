#include "corba/ws/MakeHumanHandler.hh"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <string>
#include <set>
#include <iostream>

// using namespace std clashes with bind(), so we have to be picky...
using std::set, std::string, std::vector, std::cout, std::cerr, std::endl;

static void chordataInit();
static void hexdump(unsigned char *buffer, int received);

extern set<EventHandler *> handlers;

static char* chordata_buf = nullptr;
static ssize_t chordata_len = 0;

extern void chordataLoop() {
    wsInit();
    chordataInit();

    while (true) {
        wsHandle(true);
        if (isChordataRequested() && chordata_buf) {
            // printf("chordata send %zd octets\n", chordata_len);
            // hexdump((unsigned char*)chordata_buf, chordata_len);
            sendChordata(chordata_buf, chordata_len);
            chordata_buf = nullptr;
        }
    }
}

class ChordataRecvHandler : public EventHandler {
    public:
        ChordataRecvHandler(int fd) : fd_(fd) {}
        virtual ~ChordataRecvHandler();
        virtual int on_read_event();
        virtual int on_write_event() { return 0; }
        virtual bool want_read() { return true; }
        virtual bool want_write() { return false; }
        virtual int fd() const { return fd_; }
        virtual const char *name() const { return "HttpHandshakeRecvHandler"; }
        virtual bool finish() { return !accept_key_.empty(); }
        virtual EventHandler *next() { return nullptr; }

    private:
        int fd_;
        string headers_;
        string accept_key_;
};

ChordataRecvHandler::~ChordataRecvHandler() {
    cout << "ChordataRecvHandler: close" << endl;
    shutdown(fd_, SHUT_WR);
    close(fd_);
}

int ChordataRecvHandler::on_read_event() {
    sockaddr_in client;
    static char buf[4096];
    socklen_t client_address_size = sizeof(client);
    ssize_t len = recvfrom(fd_, buf, sizeof(buf), 0, (struct sockaddr *)&client, &client_address_size);
    if (len < 0) {
        perror("recvfrom");
        return 1;
    }
    // hexdump((unsigned char *)buf, len);
    chordata_buf = buf;
    chordata_len = len;
    // cout << "chordata rcvd " << len << " octets" << endl;
    return 0;
}

// Notochord is setup to set COOP to this host as UDP port 6565
void chordataInit() {
    // open chordata UDP port // 192.168.178.24
    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    int yes = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

    sockaddr_in name;
    name.sin_family = AF_INET;
    name.sin_addr.s_addr = htonl(INADDR_ANY);
    name.sin_port = htons(6565);

    if (bind(sock, (sockaddr *)&name, sizeof(sockaddr_in)) < 0) {
        perror("bind");
        close(sock);
        sock = -1;
        exit(1);
    }

    handlers.insert(new ChordataRecvHandler(sock));
}

void hexdump(unsigned char *buffer, int received) {
    int data = 0;
    while (data < received) {
        for (int x = 0; x < 16; x++) {
            if (data < received)
                printf("%02x ", (int)buffer[data]);
            else
                printf("   ");
            data++;
        }
        data -= 16;
        for (int x = 0; x < 16; x++) {
            if (data < received)
                printf("%c", buffer[data] >= 32 && buffer[data] <= 127 ? buffer[data] : '.');
            else
                printf(" ");
            data++;
        }
        printf("\n");
    }
}
