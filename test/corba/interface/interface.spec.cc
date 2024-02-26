#include "interface.hh"

#include "../../fake.hh"
#include "interface_skel.hh"
#include "interface_stub.hh"
#include "kaffeeklatsch.hh"

using namespace std;
using namespace kaffeeklatsch;
using CORBA::async, CORBA::ORB;

class Interface_impl : public Interface_skel {
    public:
        Interface_impl(ORB *orb) : Interface_skel(orb) {}
        async<bool> callBoolean(bool value) override { co_return value; }
        async<uint8_t> callOctet(uint8_t value) override { co_return value; }  // check uint8_t with real CORBA
        async<uint16_t> callUShort(uint16_t value) override { co_return value; }
        async<uint32_t> callUnsignedLong(uint32_t value) override { co_return value; }
        async<uint64_t> callUnsignedLongLong(uint64_t value) override { co_return value; }
        // async<string> callString(string_view value) override { co_return string(value); }
};

kaffeeklatsch_spec([] {
    describe("interface", [] {
        fit("send'n receive", [] {
            // SERVER
            auto serverORB = make_shared<ORB>();
            auto serverProtocol = new FakeTcpProtocol(serverORB.get(), "backend.local", 2809);
            serverORB->registerProtocol(serverProtocol);
            auto backend = make_shared<Interface_impl>(serverORB.get());
            serverORB->bind("Backend", backend);

            // CLIENT
            auto clientORB = make_shared<ORB>();
            auto clientProtocol = new FakeTcpProtocol(clientORB.get(), "frontend.local", 32768);
            clientORB->registerProtocol(clientProtocol);

            std::exception_ptr eptr;

            [&clientORB]() -> async<> {
                auto object = co_await clientORB->stringToObject("corbaname::backend.local:2809#Backend");
                auto backend = Interface::_narrow(object);
                expect(co_await backend->callBoolean(true)).to.equal(true);
                expect(co_await backend->callOctet(42)).to.equal(42);
                expect(co_await backend->callUShort(65535)).to.equal(65535);
                expect(co_await backend->callUnsignedLong(4294967295ul)).to.equal(4294967295ul);
                expect(co_await backend->callUnsignedLongLong(18446744073709551615ull)).to.equal(18446744073709551615ull);
            }()
                                  .thenOrCatch([] {},
                                               [&eptr](std::exception_ptr _eptr) {
                                                   eptr = _eptr;
                                               });

            vector<FakeTcpProtocol *> protocols = {serverProtocol, clientProtocol};
            while (transmit(protocols))
                ;

            if (eptr) {
                std::rethrow_exception(eptr);
            }
        });
    });
});