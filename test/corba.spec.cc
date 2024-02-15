import std;

#include "../src/corba/corba.hh"

#include "../src/corba/orb.hh"
#include "../src/corba/url.hh"
#include "util.hh"

#include "kaffeeklatsch.hh"

#include <charconv>

using namespace kaffeeklatsch;

using namespace std;

struct FakeTcpProtocol : public CORBA::detail::Protocol {
        CORBA::task<CORBA::detail::Connection *> connect(const CORBA::ORB *orb, const std::string &hostname, uint16_t port);
        CORBA::task<void> close();

        static void *buffer;
        static size_t size;
};

class TcpFakeConnection : public CORBA::detail::Connection {
        std::string m_localAddress;
        uint16_t m_localPort;
        std::string m_remoteAddress;
        uint16_t m_remotePort;

    public:
        TcpFakeConnection(const string &remoteAddress, uint16_t remotePort)
            : m_localAddress("localhost"), m_localPort(1111), m_remoteAddress(remoteAddress), m_remotePort(remotePort) {}

        std::string localAddress() const { return m_localAddress; }
        uint16_t localPort() const { return m_localPort; }
        std::string remoteAddress() const { return m_remoteAddress; }
        uint16_t remotePort() const { return m_remotePort; }

        void close();
        void send(void *buffer, size_t nbyte);
};

CORBA::task<CORBA::detail::Connection *> FakeTcpProtocol::connect(const ::CORBA::ORB *orb, const std::string &hostname, uint16_t port) {
    println("TcpFakeConnection::connect(\"{}\", {})", hostname, port);
    co_return new TcpFakeConnection(hostname, port);
}

CORBA::task<void> FakeTcpProtocol::close() { co_return; }

void *FakeTcpProtocol::buffer = nullptr;
size_t FakeTcpProtocol::size = 0;

void TcpFakeConnection::close() {}
void TcpFakeConnection::send(void *buffer, size_t nbyte) {
    println("TcpFakeConnection::send(...)");
    if (FakeTcpProtocol::buffer) {
        free(FakeTcpProtocol::buffer);
    }
    FakeTcpProtocol::buffer = malloc(nbyte);
    memcpy(FakeTcpProtocol::buffer, buffer, nbyte);
    FakeTcpProtocol::size = nbyte;
}

class Backend_skel : public CORBA::Skeleton {
    public:
        Backend_skel(CORBA::ORB *orb) : Skeleton(orb) {}
};

class Backend_impl : public Backend_skel {
    public:
        Backend_impl(CORBA::ORB *orb) : Backend_skel(orb) {}

    protected:
        void _call(const std::string_view &operation, CORBA::GIOPDecoder &decoder, CORBA::GIOPEncoder &encoder);
};

void Backend_impl::_call(const std::string_view &operation, CORBA::GIOPDecoder &decoder, CORBA::GIOPEncoder &encoder) {}

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
        fit("play ping pong", [] {
            auto protocol = new FakeTcpProtocol();

            // SERVER
            auto serverORB = make_shared<CORBA::ORB>();
            serverORB->registerProtocol(protocol);
            serverORB->bind("Backend", make_shared<Backend_impl>(serverORB.get()));

            // CLIENT
            auto clientORB = make_shared<CORBA::ORB>();
            clientORB->registerProtocol(protocol);

            auto task = [clientORB]() -> CORBA::task<> {
                println("STEP 0");
                int num = co_await clientORB->stringToObject("corbaname::localhost:9001#Backend");
                println("STEP 1: GOT {}", num);
                co_return;
            }();
            task.no_wait();
            println("TO RECEIVER");
            serverORB->_socketRcvd((const uint8_t *)FakeTcpProtocol::buffer, FakeTcpProtocol::size);
        });
    });
});
