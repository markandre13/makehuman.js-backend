#pragma once

#include <corba/protocol.hh>

class TcpFakeConnection;

struct FakeTcpProtocol : public CORBA::detail::Protocol {
        FakeTcpProtocol(const std::string &localAddress, uint16_t localPort) : m_localAddress(localAddress), m_localPort(localPort) {}

        CORBA::detail::Connection *connect(const CORBA::ORB *orb, const std::string &hostname, uint16_t port);
        CORBA::async<void> close();

        std::string m_localAddress;
        uint16_t m_localPort;

        static TcpFakeConnection *sender;
        static void *buffer;
        static size_t size;
};

class TcpFakeConnection : public CORBA::detail::Connection {
        std::string m_localAddress;
        uint16_t m_localPort;
        std::string m_remoteAddress;
        uint16_t m_remotePort;

    public:
        TcpFakeConnection(const std::string &localAddress, uint16_t localPort, const std::string &remoteAddress, uint16_t remotePort)
            : m_localAddress(localAddress), m_localPort(localPort), m_remoteAddress(remoteAddress), m_remotePort(remotePort) {}

        std::string localAddress() const { return m_localAddress; }
        uint16_t localPort() const { return m_localPort; }
        std::string remoteAddress() const { return m_remoteAddress; }
        uint16_t remotePort() const { return m_remotePort; }

        void close();
        void send(void *buffer, size_t nbyte);
};
