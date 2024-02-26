#include "interface.hh"

#include "../../fake.hh"
#include "interface_skel.hh"
#include "interface_stub.hh"
#include "kaffeeklatsch.hh"

using namespace std;
using namespace kaffeeklatsch;

class Interface_impl : public Interface_skel {
    public:
        Interface_impl(CORBA::ORB *orb) : Interface_skel(orb) {}
        virtual CORBA::async<bool> callBoolean(bool value) { co_return value; }
};

kaffeeklatsch_spec([] {
    describe("interface", [] {
        fit("send'n receive", [] {
            // SERVER
            auto serverORB = make_shared<CORBA::ORB>();
            auto serverProtocol = new FakeTcpProtocol(serverORB.get(), "backend.local", 8080);
            serverORB->registerProtocol(serverProtocol);
            auto backend = make_shared<Interface_impl>(serverORB.get());
            serverORB->bind("Backend", backend);

            // CLIENT
            auto clientORB = make_shared<CORBA::ORB>();
            auto clientProtocol = new FakeTcpProtocol(clientORB.get(), "frontend.local", 32768);
            clientORB->registerProtocol(clientProtocol);

            bool success = false;

            [&]() -> CORBA::async<> {
                auto object = co_await clientORB->stringToObject("corbaname::backend.local:8080#Backend");
                auto backend = Interface::_narrow(object);
                expect(co_await backend->callBoolean(true)).to.equal(true);
                success = true;
                co_return;
            }()
                         .no_wait();

            println("REQUEST TO BACKEND resolve_str() ================================================");
            std::vector<FakeTcpProtocol *> protocols = {serverProtocol, clientProtocol};
            while(transmit(protocols));

            expect(success).to.beTrue();
        });
    });
});
