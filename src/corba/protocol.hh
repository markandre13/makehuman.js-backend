#pragma once

#include <string>
#include <map>

#include "coroutine.hh"

namespace CORBA {

class ORB;
class Stub;
class GIOPDecoder;

namespace detail {

// Protocol provides an interface to the ORB for various network protocols, eg.
// orb.registerProtocol(new TcpProtocol(orb))
// orb.registerProtocol(new WebSocketProtocol(orb))
class Connection {
    public:
        ORB *orb;

        // request id's are per connection
        uint32_t requestId = 0;

        interlock<uint32_t, GIOPDecoder *> interlock;

        // bi-directional service context needs only to be send once
        bool didSendBiDirIIOP = false;

        // stubs may contain OID received via this connection
        // TODO: WeakMap? refcount tests
        // objectId to stub?
        std::map<std::string, Stub *> stubsById;

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

class Protocol {
    public:
        // hm... do we return something or do we call ORB?
        virtual Connection *connect(const ORB *orb, const std::string &hostname, uint16_t port) = 0;
        virtual task<void> close() = 0;
};

}  // namespace detail

}  // namespace CORBA
