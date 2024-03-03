#pragma once

#include "../protocol.hh"
#include <ev.h>

struct client_handler_t;

class MyConnection : public CORBA::detail::Connection {
        std::string m_localAddress;
        uint16_t m_localPort;
        std::string m_remoteAddress;
        uint16_t m_remotePort;

    public:
        MyConnection(const std::string &localAddress, uint16_t localPort, const std::string &remoteAddress, uint16_t remotePort, uint32_t initialRequestId = 0)
            : Connection(initialRequestId), m_localAddress(localAddress), m_localPort(localPort), m_remoteAddress(remoteAddress), m_remotePort(remotePort) {}

        const std::string & localAddress() const override { return m_localAddress; }
        uint16_t localPort() const override { return m_localPort; }
        const std::string & remoteAddress() const override { return m_remoteAddress; }
        uint16_t remotePort() const override { return m_remotePort; }

        void close() override;
        void send(void *buffer, size_t nbyte) override;

        client_handler_t * handler;
};

struct MyProtocol : public CORBA::detail::Protocol {
        std::string m_localAddress;
        uint16_t m_localPort;

        void listen(CORBA::ORB *orb, struct ev_loop *loop, const std::string &hostname, uint16_t port);

        MyConnection *connect(const CORBA::ORB *orb, const std::string &hostname, uint16_t port) override;
        CORBA::async<void> close() override;
};
