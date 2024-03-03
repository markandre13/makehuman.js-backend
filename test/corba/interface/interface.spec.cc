#include "interface.hh"

#include "../../fake.hh"
#include "interface_skel.hh"
#include "interface_stub.hh"
#include "kaffeeklatsch.hh"

namespace cppasync {

#ifdef _COROUTINE_DEBUG
unsigned promise_sn_counter = 0;
unsigned async_sn_counter = 0;
unsigned awaitable_sn_counter = 0;
unsigned promise_use_counter = 0;
unsigned async_use_counter = 0;
unsigned awaitable_use_counter = 0;
#endif

}  // namespace cppasync

using namespace std;
using namespace kaffeeklatsch;
using CORBA::async, CORBA::ORB, CORBA::blob, CORBA::blob_view;

class Interface_impl : public Interface_skel {
    public:
        Interface_impl(std::shared_ptr<CORBA::ORB> orb) : Interface_skel(orb) {}
        async<bool> callBoolean(bool value) override { co_return value; }
        async<uint8_t> callOctet(uint8_t value) override { co_return value; }  // check uint8_t with real CORBA
        async<uint16_t> callUShort(uint16_t value) override { co_return value; }
        async<uint32_t> callUnsignedLong(uint32_t value) override { co_return value; }
        async<uint64_t> callUnsignedLongLong(uint64_t value) override { co_return value; }
        async<string> callString(const string_view &value) override { co_return string(value); }
        async<blob> callBlob(const blob_view &value) override { co_return blob(value); }

        std::shared_ptr<Peer> peer;
        async<void> setPeer(std::shared_ptr<Peer> aPeer) override {
            this->peer = aPeer;
            co_return;
        }
        async<std::string> callPeer(const std::string_view &value) override {
            // println("INTERFACE IMPL: RECEIVED callPeer(\"{}\"): call peer", value);
            auto s = co_await peer->callString(string(value) + " to the");
            // value is not valid anymore
            // println("INTERFACE IMPL: RECEIVED callPeer(...): peer returned \"{}\", return value \"{}.\"", s, s);
            co_return s + ".";
        }
        // next steps:
        // [X] set/get callback object and call it
        // [ ] use ArrayBuffer/Buffer for sequence<octet> for the javascript side
        // [ ] completeness: signed & floating point
};

class Peer_impl : public Peer_skel {
    public:
        Peer_impl(std::shared_ptr<CORBA::ORB> orb) : Peer_skel(orb) {}
        async<string> callString(const string_view &value) override { co_return string(value) + " world"; }
};

// due to
//
//   CP.51: Do not use capturing lambdas that are coroutines.
//   https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rcoro-capture
//
// the following causes a memory error when accessing 'orb' after the co_await
//
//   [&orb] -> async<> { ... co_await ... }().thenOrCatch(...)
//
// as the closure object will be destroyed after the co_await.
//
// this on the other hand works with clang:
//
//   auto call(std::function<async<>()> closure) { return closure(); }
//   call([&orb] -> async<> { ... co_await ... }).thenOrCatch(...)
//
// and i assume it's because then the compiler has to pass the closure forward
// into the call() function and then it's lifetime get's intertwined with the
// coroutine.
void parallel(std::exception_ptr &eptr, std::function<CORBA::async<>()> closure) {
    closure().thenOrCatch([] {},
                          [&eptr](std::exception_ptr _eptr) {
                              eptr = _eptr;
                          });
}

kaffeeklatsch_spec([] {
    describe("interface", [] {
        it("send'n receive", [] {
            // SERVER
            auto serverORB = make_shared<ORB>();
            auto serverProtocol = new FakeTcpProtocol(serverORB.get(), "backend.local", 2809);
            serverORB->registerProtocol(serverProtocol);
            // serverORB->debug = true;
            auto backend = make_shared<Interface_impl>(serverORB);
            serverORB->bind("Backend", backend);

            // CLIENT
            auto clientORB = make_shared<ORB>();
            // clientORB->debug = true;
            auto clientProtocol = new FakeTcpProtocol(clientORB.get(), "frontend.local", 32768);
            clientORB->registerProtocol(clientProtocol);

            std::exception_ptr eptr;

            parallel(eptr, [&clientORB] -> async<> {
                auto object = co_await clientORB->stringToObject("corbaname::backend.local:2809#Backend");
                auto backend = Interface::_narrow(object);
                expect(co_await backend->callBoolean(true)).to.equal(true);
                expect(co_await backend->callOctet(42)).to.equal(42);
                expect(co_await backend->callUShort(65535)).to.equal(65535);
                expect(co_await backend->callUnsignedLong(4294967295ul)).to.equal(4294967295ul);
                expect(co_await backend->callUnsignedLongLong(18446744073709551615ull)).to.equal(18446744073709551615ull);
                expect(co_await backend->callString("hello")).to.equal("hello");
                expect(co_await backend->callBlob(blob_view("hello"))).to.equal(blob("hello"));
                auto frontend = make_shared<Peer_impl>(clientORB);
                co_await backend->setPeer(frontend);
                expect(co_await backend->callPeer("hello")).to.equal("hello to the world.");
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