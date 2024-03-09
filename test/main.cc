// import std;
// using namespace std;

#include "kaffeeklatsch.hh"
using namespace kaffeeklatsch;

int main(int argc, char *argv[]) { return kaffeeklatsch::run(argc, argv); }

// #include <unistd.h>

// #include <print>
// // socket
// #include <sys/socket.h>
// // htonl
// #include <arpa/inet.h>
// // gethostbyaddr
// #include <netdb.h>
// #include <sys/param.h>

// // #include <netinet/in.h>

// // this collides with macos x headers...
// // #include <ossp/uuid.h>
// #include <uuid/uuid.h>

// using namespace std;

// #include <netinet/in.h>

// server
// ------
// 2809 is the default port
// listen(hostname, port)       // will listen on hostname (get's host name)
// listen(any, port) // will listen on all inet/inet6 interfaces (uses NULL)
// listen(loopback) or listen("127.0.0.1") or .. (ipv6 localhost)
// The loopback address in IPv4 is 127.0.01. In IPv6, the loopback address is 0:0:0:0:0:0:0:1 or ::1.
//
// once a connection has been created...
//
// client
// ------
// [ ] need to find out peer and own address
//
// how it works


// [ ] can an orb send it's hostname & ip?
//   [ ] service context so far: bi_dir_iiop with a hostname & port (current receive only...)
//   [ ] CORBA 3.4 Part 2, 9.8.1 Bi-directional IIOP Service Context
//       * since GIOP 1.2
//       * scenario: client sends IOR to server, so that the server can call the client
//       * client sends request containing BiDirIIOPServiceContext with a hostname and port 
//         (actually, it's a list of host:port pairs. and if multiple BiDirIIOPServiceContext's
//         are send, the latter are added to the previous ones)
//         (client could either use it's host and ip... but that could be private... and collide
//         with other private names... standard says host can be anything in this case. so the `uuid-{uuid}.corba`
//         idea seems to be allowed
//       * i haven't implemented the policy stuff, but then with omniorb is already worked without it.
//       * server uses that connection when it needs to contact that hostname and port
// o   when a server sends an ior, it uses it's public address
// o there is a closeconnection message...


// static const int yes = 1;

// int main() {
//     uuid_t uuid;

//     // f81a9373-ef88-41e9-bea5-f1ad095f3811

//     uuid_string_t uuid_str;
//     uuid_generate_random(uuid);
//     uuid_unparse_lower(uuid, uuid_str);
//     println("UUID 'uuid-{}.corba'", uuid_str);

//     struct addrinfo hints, *res, *res0;
//     int error;
//     //    int s[MAXSOCK];
//     int nsock;
//     const char *cause = NULL;

//     char hostname[MAXHOSTNAMELEN];
//     gethostname(hostname, sizeof(hostname));
//     printf("hostname = '%s'\n", hostname);

//     // null + AI_PASSIVE -> any (canonname = localhost)
//     // null + 0 -> loopback

//     // null -> canonname = localhost
//     // hostname -> canonname = hostname
//     // 192.168.178.24 -> "(null)"

//     memset(&hints, 0, sizeof(hints));
//     hints.ai_family = PF_UNSPEC;
//     hints.ai_socktype = SOCK_STREAM;
//     hints.ai_flags = AI_PASSIVE | AI_CANONNAME; // AI_PASSIVE: suitable for bind/accept
//     error = getaddrinfo("192.168.178.24", "1", &hints, &res0);  // nullptr means 0.0.0.0
//     if (error) {
//         printf("%s", gai_strerror(error));
//         return 0;
//         /*NOTREACHED*/
//     }
//     nsock = 0;
//     for (res = res0; res; res = res->ai_next) {
//         switch (res->ai_family) {
//             case AF_INET: {
//                 auto addr = (sockaddr_in *)res->ai_addr;
//                 char buffer[INET_ADDRSTRLEN];
//                 inet_ntop(res->ai_family, &addr->sin_addr, buffer, INET_ADDRSTRLEN);
//                 printf("ipv4 %s:%i (%s)\n", buffer, (int)ntohs(addr->sin_port), res->ai_canonname);
//             } break;
//             case AF_INET6: {
//                 auto addr = (sockaddr_in6 *)res->ai_addr;
//                 char buffer[INET6_ADDRSTRLEN];
//                 inet_ntop(res->ai_family, &addr->sin6_addr, buffer, INET6_ADDRSTRLEN);
//                 printf("ipv6 [%s]:%u (%s)\n", buffer, ntohs(addr->sin6_port), res->ai_canonname);
//             } break;
//             default:
//                 printf("unknown\n");
//         }
//         //    s[nsock] = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
//         //    if (s[nsock] < 0) {
//         //            cause = "socket";
//         //            continue;
//         //    }

//         //    if (bind(s[nsock], res->ai_addr, res->ai_addrlen) < 0) {
//         //            cause = "bind";
//         //            close(s[nsock]);
//         //            continue;
//         //    }
//         //    (void) listen(s[nsock], 5);

//         nsock++;
//     }
//     if (nsock == 0) {
//         printf("%s", cause);
//         return 0;
//         /*NOTREACHED*/
//     }
//     freeaddrinfo(res0);

    // int fd = socket(PF_INET, SOCK_STREAM, 0);
    // if (fd == -1) {
    //     perror("socket");
    //     return 1;
    // }

    // setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

    // sockaddr_in local_addr;
    // local_addr.sin_family = AF_INET;
    // local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    // // local_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    // local_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    // local_addr.sin_port = htons(2809);

    // if (bind(fd, (sockaddr *)&local_addr, (socklen_t)sizeof(local_addr)) == -1) {
    //     perror("bind");
    //     close(fd);
    //     return 1;
    // }

    // if (listen(fd, 20) == -1) {
    //     perror("listen tcp");
    //     return 1;
    // }

    // {
    //     struct sockaddr_in sin;
    //     socklen_t len = sizeof(sin);
    //     if (getsockname(fd, (struct sockaddr *)&sin, &len) == -1) {
    //         perror("getsockname");
    //     } else {
    //         println("server socket open at {}:{}", inet_ntoa(sin.sin_addr), ntohs(sin.sin_port));
    //     }
    // }

    // while (true) {
    //     println("wait for client");
    //     sockaddr_in remote_addr;
    //     socklen_t len = sizeof(remote_addr);
    //     int fd0 = accept(fd, (struct sockaddr *)&remote_addr, &len);

    //     struct hostent *hp = gethostbyaddr(&remote_addr.sin_addr, len, remote_addr.sin_family);

    //     printf("tcp: got connection from %s:%d on", hp ? hp->h_name : inet_ntoa(remote_addr.sin_addr), ntohs(remote_addr.sin_port));
    //     printf(" %s:%d\n", inet_ntoa(local_addr.sin_addr), ntohs(local_addr.sin_port));

    //     close(fd0);
    // }
//     return 0;
// }