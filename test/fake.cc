#include "fake.hh"

using std::println;

CORBA::detail::Connection *FakeTcpProtocol::connect(const ::CORBA::ORB *orb, const std::string &hostname, uint16_t port) {
    println("TcpFakeConnection::connect(\"{}\", {})", hostname, port);
    auto conn = new TcpFakeConnection(m_localAddress, m_localPort, hostname, port);
    printf("TcpFakeConnection::connect() -> %p %s:%u -> %s:%u requestId=%u\n", static_cast<void *>(conn), conn->localAddress().c_str(), conn->localPort(),
           conn->remoteAddress().c_str(), conn->remotePort(), conn->requestId);
    return conn;
}

CORBA::async<void> FakeTcpProtocol::close() { co_return; }

TcpFakeConnection *FakeTcpProtocol::sender;
void *FakeTcpProtocol::buffer = nullptr;
size_t FakeTcpProtocol::size = 0;

void TcpFakeConnection::close() {}
void TcpFakeConnection::send(void *buffer, size_t nbyte) {
    println("TcpFakeConnection::send(...) from {}:{} to {}:{}", m_localAddress, m_localPort, m_remoteAddress, m_remotePort);
    FakeTcpProtocol::sender = this;
    if (FakeTcpProtocol::buffer) {
        free(FakeTcpProtocol::buffer);
    }
    FakeTcpProtocol::buffer = malloc(nbyte);
    memcpy(FakeTcpProtocol::buffer, buffer, nbyte);
    FakeTcpProtocol::size = nbyte;
}
