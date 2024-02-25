import std;

#include "../src/corba/corba.hh"

#include <charconv>
#include <cstring>
#include <format>

#include "../src/corba/orb.hh"
#include "../src/corba/protocol.hh"
#include "../src/corba/url.hh"
#include "kaffeeklatsch.hh"
#include "makehuman.hh"
#include "makehuman_skel.hh"
#include "makehuman_stub.hh"
#include "util.hh"

using namespace kaffeeklatsch;

using namespace std;

class TcpFakeConnection;

struct FakeTcpProtocol : public CORBA::detail::Protocol {
        FakeTcpProtocol(const string &localAddress, uint16_t localPort) : m_localAddress(localAddress), m_localPort(localPort) {}

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
        TcpFakeConnection(const string &localAddress, uint16_t localPort, const string &remoteAddress, uint16_t remotePort)
            : m_localAddress(localAddress), m_localPort(localPort), m_remoteAddress(remoteAddress), m_remotePort(remotePort) {}

        std::string localAddress() const { return m_localAddress; }
        uint16_t localPort() const { return m_localPort; }
        std::string remoteAddress() const { return m_remoteAddress; }
        uint16_t remotePort() const { return m_remotePort; }

        void close();
        void send(void *buffer, size_t nbyte);
};

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

class Backend_impl : public Backend_skel {
    public:
        Backend_impl(CORBA::ORB *orb) : Backend_skel(orb) {}
        virtual CORBA::async<string> hello(string word) override {
            println("Backend_impl::hello(\"{}\")", word);
            co_return word + " world.";
        }
        virtual CORBA::async<> fail() override {
            throw CORBA::BAD_OPERATION(0, CORBA::YES);
            co_return;
        }
};

kaffeeklatsch_spec([] {
    describe("URL", [] {
        // describe("URLLexer", [] {
        //     describe("number()", [] {
        //         it("returns a number on success", [] {
        //             UrlLexer url("123a");
        //             expect(url.number()).to.equal(make_optional(123));
        //             // expect(url.pos).to.equal(3)
        //         });
        //         it("returns undefined on failure", [] {
        //             UrlLexer url("a123");
        //             expect(url.number()).to.be.undefined();
        //             expect(url.pos).to.equal(0);
        //         });
        //     });
        //     describe("match(string)", [] {
        //         it("is a match", []() {
        //             UrlLexer url("foobar");
        //             expect(url.match("foo")).to.beTrue();
        //             expect(url.pos).to.equal(3);
        //         });
        //         it("is a match till end of url", [] {
        //             UrlLexer url("foo");
        //             expect(url.match("foo")).to.beTrue();
        //             expect(url.pos).to.equal(3);
        //         });
        //         it("is no match", [] {
        //             UrlLexer url("foobar");
        //             expect(url.match("fu")).to.beFalse();
        //             expect(url.pos).to.equal(0);
        //         });
        //         it("is no match after end of url", [] {
        //             UrlLexer url("foobar");
        //             expect(url.match("foobart")).to.beFalse();
        //             expect(url.pos).to.equal(0);
        //         });
        //     });
        // });
        describe("variant<IOR, CorbaLocation, CorbaName> decodeURI(const string &uri)", [] {
            it("ior", [] {
                auto ior = CORBA::decodeURI(
                    "IOR:"
                    "010000000f00000049444c3a5365727665723a312e30000002000000000000002f000000010100000e0000003139322e3136382e312e313035002823130000002f3131"
                    "3031"
                    "2f313632363838383434312f5f30000100000024000000010000000100000001000000140000000100000001000100000000000901010000000000");
                expect([&] {
                    std::get<CORBA::IOR>(ior);
                })
                    .to.not_()
                    .throw_();
            });
            describe("corbaloc", [] {
                describe("iiop", [] {
                    it("defaults", [] {
                        auto uri = CORBA::decodeURI("corbaloc::mark13.org/Prod/TradingService");
                        expect(std::get<CORBA::CorbaLocation>(uri).str()).to.equal("corbaloc:iiop:1.0@mark13.org:2809/Prod/TradingService");
                    });
                    it("IPv4", [] {
                        auto uri = CORBA::decodeURI("corbaloc::127.0.0.1/Prod/TradingService");
                        expect(std::get<CORBA::CorbaLocation>(uri).str()).to.equal("corbaloc:iiop:1.0@127.0.0.1:2809/Prod/TradingService");
                    });
                    it("IPv6", [] {
                        auto uri = CORBA::decodeURI("corbaloc::[1080::8:800:200C:417A]/Prod/TradingService");
                        expect(std::get<CORBA::CorbaLocation>(uri).str()).to.equal("corbaloc:iiop:1.0@[1080::8:800:200C:417A]:2809/Prod/TradingService");
                    });
                    it("3 hostnames", [] {
                        auto uri = CORBA::decodeURI("corbaloc:iiop:1.1@mark-13.org:8080,iiop:1.2@mhsd.de:4040,:dawnrazor.co.uk/Prod/TradingService");
                        expect(std::get<CORBA::CorbaLocation>(uri).str())
                            .to.equal("corbaloc:iiop:1.1@mark-13.org:8080,iiop:1.2@mhsd.de:4040,iiop:1.0@dawnrazor.co.uk:2809/Prod/TradingService");
                    });
                });
                describe("rir", [] {
                    it("rir", [] {
                        auto uri = CORBA::decodeURI("corbaloc:rir:/NameService");
                        expect(std::get<CORBA::CorbaLocation>(uri).str()).to.equal("corbaloc:rir:/NameService");
                    });
                });
            });
            describe("corbaname", [] {
                it("iiop", [] {
                    auto uri = CORBA::decodeURI("corbaname::555objs.com#a/string/path/to/obj");
                    expect(std::get<CORBA::CorbaName>(uri).str()).to.equal("corbaname:iiop:1.0@555objs.com:2809/NameService#a/string/path/to/obj");
                });
                it("rir", [] {
                    auto uri = CORBA::decodeURI("corbaname:rir:#a/local/obj");
                    expect(std::get<CORBA::CorbaName>(uri).str()).to.equal("corbaname:rir:/NameService#a/local/obj");
                });
            });
        });
    });
    describe("IOR(string) (CORBA 3.3, 7.6.2 Interoperable Object References)", [] {
        it("error when not prefixed with 'IOR:'", [] {
            expect([] {
                // prettier-ignore
                CORBA::IOR ior("x");
            }).to.throw_(runtime_error(R"(Missing "IOR:" prefix in "x".)"));
        });
        it("error when size is not a multiple of 2", [] {
            expect([] {
                // prettier-ignore
                CORBA::IOR ior("IOR:x");
            }).to.throw_(runtime_error(R"(IOR has a wrong length.)"));
        });
        it("get host, port, oid and objectKey from IOR", [] {
            CORBA::IOR ior(
                "IOR:"
                "010000000f00000049444c3a5365727665723a312e30000002000000000000002f000000010100000e0000003139322e3136382e312e313035002823130000002f313130312f31"
                "3632363838383434312f5f30000100000024000000010000000100000001000000140000000100000001000100000000000901010000000000");
            expect(ior.host).to.equal("192.168.1.105");
            expect(ior.port).to.equal(9000);
            expect(ior.oid).to.equal("IDL:Server:1.0");
            expect(ior.objectKey).to.equal("/1101/1626888441/_0");
        });
    });
    describe("integration tests", [] {
        it("play ping pong", [] {
            // SERVER
            auto serverORB = make_shared<CORBA::ORB>();
            auto serverProtocol = new FakeTcpProtocol("backend.local", 8080);
            serverORB->registerProtocol(serverProtocol);

            auto backend = make_shared<Backend_impl>(serverORB.get());

            serverORB->bind("Backend", backend);

            // CLIENT

            auto clientORB = make_shared<CORBA::ORB>();
            auto clientProtocol = new FakeTcpProtocol("frontend.local", 32768);
            clientORB->registerProtocol(clientProtocol);

            [&]() -> CORBA::async<> {
                println("STEP 0: RESOLVE OBJECT");
                auto object = co_await clientORB->stringToObject("corbaname::backend.local:8080#Backend");
                println("STEP 1: GOT OBJECT");
                auto backend = Backend::_narrow(object);
                println("STEP 2: CALL OBJECT");
                auto reply = co_await backend->hello("hello");
                cerr << "GOT " << reply << endl;
                println("STEP 3: CALL OBJECT AND GET EXCEPTION");
                try {
                    co_await backend->fail();
                } catch (CORBA::SystemException &ex) {
                    println("caught exception {}", ex.major());
                }
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
                println("REQUEST TO BACKEND fail() ================================================");
                co_await serverORB->_socketRcvd(serverConn, (const uint8_t *)FakeTcpProtocol::buffer, FakeTcpProtocol::size);
                println("REPLY TO FRONTEND fail() =================================================");
                co_await clientORB->_socketRcvd(clientConn, (const uint8_t *)FakeTcpProtocol::buffer, FakeTcpProtocol::size);
            }()
                         .no_wait();
        });
    });

    describe("c++ playground", [] {
        // the thing about c++ smart pointers is that they do the reference counting
        // in the pointer, not the object.
        // * the advantage of this is that shared_ptr can be used with all objects.
        // * the disadvantage is that the object itself does not know it's own shared_ptr.
        //   inheriting from std::enable_shared_from_this add's the method shared_from_this(),
        //   which returns the objects shared_ptr. BUT it can only be called when there already
        //   is at least one shared_ptr for the object available.
        it("shared_ptr", [] {
            class A : public std::enable_shared_from_this<A> {};
            auto a = make_shared<A>();
            expect(a.use_count()).to.equal(1);
            auto b = a;
            expect(a.use_count()).to.equal(2);

            auto c = a->shared_from_this();
            expect(a.use_count()).to.equal(3);
        });
    });
});

// CORBA:
// [X] websocket protocol¸¸¸¸¸
// [X] exceptions
// [X] oneway
// [ ] generate c++ code from idl
//   [X] twoway call
//   [X] oneway call
//   [ ] object references
//   [ ] special handling for octet sequences
//   [ ] string reference instead of string for 'in'
// [ ] the shared_ptr stuff is still whacky
// [ ]
// [ ] stabilize the connection handling code
// [ ]
// [ ] value types
// [ ] std::array vs std::span
// [ ] c++11 prohibits std::string being reference counted...

// coroutine:
// [X] exceptions
