#include "interface.hh"
#include "interface_stub.hh"
#include "interface_skel.hh"
#include "../../fake.hh"

#include "kaffeeklatsch.hh"

using namespace std;
using namespace kaffeeklatsch;

class Interface_impl : public Interface_skel {
    public:
        Interface_impl(CORBA::ORB *orb) : Interface_skel(orb) {}
        virtual CORBA::async<bool> callBoolean(bool value) {
            co_return value;
        }
};

kaffeeklatsch_spec([] {
    fdescribe("interface", [] {
        it("send'n receive", [] {
            // SERVER
            auto serverORB = make_shared<CORBA::ORB>();
            auto serverProtocol = new FakeTcpProtocol("backend.local", 8080);
            serverORB->registerProtocol(serverProtocol);
            auto backend = make_shared<Interface_impl>(serverORB.get());
            serverORB->bind("Backend", backend);

            // CLIENT
            auto clientORB = make_shared<CORBA::ORB>();
            auto clientProtocol = new FakeTcpProtocol("frontend.local", 32768);
            clientORB->registerProtocol(clientProtocol);

            [&]() -> CORBA::async<> {
                println("STEP 0: RESOLVE OBJECT");
                auto object = co_await clientORB->stringToObject("corbaname::backend.local:8080#Backend");
                println("STEP 1: GOT OBJECT");
                auto backend = Interface::_narrow(object);
                println("STEP 2: CALL OBJECT");
                expect(co_await backend->callBoolean(true)).to.equal(true);
                co_return;
            }()
                         .no_wait();

            [&]() -> CORBA::async<> {
                println("REQUEST TO BACKEND resolve_str() ================================================");
                auto clientConn = FakeTcpProtocol::sender;
                auto serverConn = serverORB->getConnection(FakeTcpProtocol::sender->localAddress(), FakeTcpProtocol::sender->localPort());

                printf("clientConn %p %s:%u -> %s:%u requestId=%u\n", static_cast<void *>(clientConn), clientConn->localAddress().c_str(),
                       clientConn->localPort(), clientConn->remoteAddress().c_str(), clientConn->remotePort(), clientConn->requestId);
                printf("serverConn %p %s:%u -> %s:%u requestId=%u\n", static_cast<void *>(serverConn), serverConn->localAddress().c_str(),
                       serverConn->localPort(), serverConn->remoteAddress().c_str(), serverConn->remotePort(), serverConn->requestId);

                co_await serverORB->_socketRcvd(serverConn, (const uint8_t *)FakeTcpProtocol::buffer, FakeTcpProtocol::size);
                println("REPLY TO FRONTEND resolve_str() =================================================");
                co_await clientORB->_socketRcvd(clientConn, (const uint8_t *)FakeTcpProtocol::buffer, FakeTcpProtocol::size);
                println("REQUEST TO BACKEND hello() ================================================");
                co_await serverORB->_socketRcvd(serverConn, (const uint8_t *)FakeTcpProtocol::buffer, FakeTcpProtocol::size);
                println("REPLY TO FRONTEND hello() =================================================");
                co_await clientORB->_socketRcvd(clientConn, (const uint8_t *)FakeTcpProtocol::buffer, FakeTcpProtocol::size);
                // println("REQUEST TO BACKEND fail() ================================================");
                // co_await serverORB->_socketRcvd(serverConn, (const uint8_t *)FakeTcpProtocol::buffer, FakeTcpProtocol::size);
                // println("REPLY TO FRONTEND fail() =================================================");
                // co_await clientORB->_socketRcvd(clientConn, (const uint8_t *)FakeTcpProtocol::buffer, FakeTcpProtocol::size);
            }()
                         .no_wait();
        });
    });
});
