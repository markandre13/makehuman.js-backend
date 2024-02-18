#pragma once

#include "../protocol.hh"

struct corba_handler_t;

namespace CORBA {

class WsConnection : public detail::Connection {
        std::string m_localAddress;
        uint16_t m_localPort;
        std::string m_remoteAddress;
        uint16_t m_remotePort;

    public:
        WsConnection(const std::string &localAddress, uint16_t localPort, const std::string &remoteAddress, uint16_t remotePort)
            : m_localAddress(localAddress), m_localPort(localPort), m_remoteAddress(remoteAddress), m_remotePort(remotePort) {}

        std::string localAddress() const override { return m_localAddress; }
        uint16_t localPort() const override { return m_localPort; }
        std::string remoteAddress() const override { return m_remoteAddress; }
        uint16_t remotePort() const override { return m_remotePort; }

        void close() override;
        void send(void *buffer, size_t nbyte) override;

        corba_handler_t * handler;
};

/**
 * WebSocket protocol adapter
 */
struct WsProtocol : public detail::Protocol {
        std::string m_localAddress;
        uint16_t m_localPort;

        WsProtocol(CORBA::ORB *orb);
        void listen(const std::string &hostname, uint16_t port);

        WsConnection *connect(const CORBA::ORB *orb, const std::string &hostname, uint16_t port) override;
        task<void> close() override;
};

}  // namespace CORBA
