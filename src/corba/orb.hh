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
    class Protocol;
    class Connection;
}

enum CompletionStatus { YES, NO, MAYBE };

class Exception: public std::exception {};

class UserException: public Exception {};

class SystemException : public Exception {
    public:
        uint32_t minor;
        CompletionStatus completed;
        SystemException(uint32_t minor, CompletionStatus completed) : minor(minor), completed(completed) {}
        virtual const char *major() const = 0;
};

class MARSHAL : public SystemException {
    public:
        MARSHAL(uint32_t minor, CompletionStatus completed) : SystemException(minor, completed) {}
        const char *major() const override { return "IDL:omg.org/CORBA/MARSHAL:1.0"; }
};

class NO_PERMISSION : public SystemException {
    public:
        NO_PERMISSION(uint32_t minor, CompletionStatus completed) : SystemException(minor, completed) {}
        const char *major() const override { return "IDL:omg.org/CORBA/NO_PERMISSION:1.0"; }
};

class BAD_PARAM : public SystemException {
    public:
        BAD_PARAM(uint32_t minor, CompletionStatus completed) : SystemException(minor, completed) {}
        const char *major() const override { return "IDL:omg.org/CORBA/BAD_PARAM:1.0"; }
};

class BAD_OPERATION : public SystemException {
    public:
        BAD_OPERATION(uint32_t minor, CompletionStatus completed) : SystemException(minor, completed) {}
        const char *major() const override { return "IDL:omg.org/CORBA/BAD_OPERATION:1.0"; }
};

class OBJECT_NOT_EXIST : public SystemException {
    public:
        OBJECT_NOT_EXIST(uint32_t minor, CompletionStatus completed) : SystemException(minor, completed) {}
        const char *major() const override { return "IDL:omg.org/CORBA/OBJECT_NOT_EXIST:1.0"; }
};

class TRANSIENT : public SystemException {
    public:
        TRANSIENT(uint32_t minor, CompletionStatus completed) : SystemException(minor, completed) {}
        const char *major() const override { return "IDL:omg.org/CORBA/TRANSIENT:1.0"; }
};

class OBJECT_ADAPTER : public SystemException {
    public:
        OBJECT_ADAPTER(uint32_t minor, CompletionStatus completed) : SystemException(minor, completed) {}
        const char *major() const override { return "IDL:omg.org/CORBA/OBJECT_ADAPTER:1.0"; }
};

class ORB : public std::enable_shared_from_this<ORB> {
        bool debug = true;
        NamingContextExtImpl * namingService = nullptr;
        // std::map<std::string, Skeleton*> initialReferences; // name to
        std::map<blob, Skeleton *> servants;  // objectId to skeleton

        uint64_t servantIdCounter = 0;

        std::vector<detail::Protocol*> protocols;
        std::vector<detail::Connection*> connections;

    public:
        detail::Connection * getConnection(std::string host, uint16_t port);
        void addConnection(detail::Connection *connection) { connections.push_back(connection); }
        ORB();
        void run();

        inline void registerProtocol(detail::Protocol *protocol) { protocols.push_back(protocol); }

        static void socketRcvd(const uint8_t *buffer, size_t size);
        void _socketRcvd(detail::Connection *connection, const void *buffer, size_t size);

        async<std::shared_ptr<Object>> stringToObject(const std::string &iorString);

        // register servant and create and assign a new objectKey
        blob_view registerServant(Skeleton *skeleton);
        // register servant with the given objectKey
        blob_view registerServant(Skeleton *skeleton, const std::string &objectKey);

        template <typename T>
        async<T> twowayCall(
            Stub *stub,
            const char *operation,
            std::function<void(GIOPEncoder &)> encode,
            std::function<T(GIOPDecoder &)> decode)
        {
            auto decoder = co_await _twowayCall(stub, operation, encode);
            co_return decode(*decoder);
        }

        // template <typename T>
        async<void> twowayCall(
            Stub *stub,
            const char *operation,
            std::function<void(GIOPEncoder &)> encode)
        {
            co_await _twowayCall(stub, operation, encode);
            co_return;
        }

        void onewayCall(
            Stub *stub,
            const char *operation,
            std::function<void(GIOPEncoder &)> encode);



        //
        // NameService
        //
        void bind(const std::string &id, std::shared_ptr<CORBA::Skeleton> const obj);

    protected:
        async<GIOPDecoder*> _twowayCall(Stub *stub, const char *operation, std::function<void(GIOPEncoder &)> encode);
};

}  // namespace CORBA
