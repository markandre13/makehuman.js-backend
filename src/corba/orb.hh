#pragma once

#include <functional>
#include <map>
#include <vector>
#include <memory>

#include "coroutine.hh"
#include "giop.hh"

namespace CORBA {

class Skeleton;
class Stub;
class NamingContextExtImpl;
class ORB;

namespace detail {

// Protocol provides an interface to the ORB for various network protocols, eg.
// orb.registerProtocol(new TcpProtocol(orb))
// orb.registerProtocol(new WebSocketProtocol(orb))
struct Connection {
        ORB *orb;

        // request id's are per connection
        uint32_t requestId = 0;

        interlock<uint32_t, GIOPDecoder*> interlock;

        // bi-directional service context needs only to be send once
        bool didSendBiDirIIOP = false;

        // stubs may contain OID received via this connection
        // TODO: WeakMap? refcount tests
        // objectId to stub?
        std::map<std::string, Stub*> stubsById;

        // replies to be send back over this connection
        // number: RequestId
        // WeakMap? refcount tests
        // pendingReplies = new Map<number, PromiseHandler>()

        // CSIv2 context tokens received by the client
        // BigInt: ContextId
        // initialContextTokens = new Map<BigInt, InitialContextToken>()

        virtual std::string localAddress() const = 0;
        virtual uint16_t localPort() const = 0;
        virtual std::string remoteAddress() const = 0;
        virtual uint16_t remotePort() const = 0;

        virtual void close() = 0;
        virtual void send(void *buffer, size_t nbyte) = 0;
};

struct Protocol {
        // hm... do we return something or do we call ORB?
        virtual Connection* connect(const ORB *orb, const std::string &hostname, uint16_t port) = 0;
        virtual task<void> close() = 0;
};

};  // namespace detail

class ORB : public std::enable_shared_from_this<ORB> {
        bool debug = true;
        NamingContextExtImpl * namingService = nullptr;
        // std::map<std::string, Skeleton*> initialReferences; // name to
        std::map<std::string, Skeleton *> servants;  // objectId to skeleton

        uint64_t servantIdCounter = 0;

        std::vector<detail::Protocol*> protocols;
        std::vector<detail::Connection*> connections;

    public:
        detail::Connection * getConnection(std::string host, uint16_t port);
        ORB();
        void run();

        inline void registerProtocol(detail::Protocol *protocol) { protocols.push_back(protocol); }

        static void socketRcvd(const uint8_t *buffer, size_t size);
        CORBA::task<> _socketRcvd(detail::Connection *connection, const uint8_t *buffer, size_t size);

        task<std::shared_ptr<Object>> stringToObject(const std::string &iorString);

        // register servant and create and assign a new objectKey
        std::string registerServant(Skeleton *skeleton);
        // register servant with the given objectKey
        std::string registerServant(Skeleton *skeleton, const std::string &objectKey);

        template <typename T>
        task<T> twowayCall(
            Stub *stub,
            const char *method,
            std::function<void(GIOPEncoder &)> encode,
            std::function<T(GIOPDecoder &)> decode)
        {
            auto decoder = co_await _twowayCall(stub, method, encode);
            co_return decode(*decoder);
        }

        //
        // NameService
        //
        void bind(const std::string &id, std::shared_ptr<CORBA::Skeleton> const obj);

    protected:
        task<GIOPDecoder*> _twowayCall(Stub *stub, const char *method, std::function<void(GIOPEncoder &)> encode);
};

}  // namespace CORBA
