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
        void addConnection(detail::Connection *connection) { connections.push_back(connection); }
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

        // template <typename T>
        task<void> twowayCall(
            Stub *stub,
            const char *method,
            std::function<void(GIOPEncoder &)> encode)
        {
            co_await _twowayCall(stub, method, encode);
            co_return;
        }

        //
        // NameService
        //
        void bind(const std::string &id, std::shared_ptr<CORBA::Skeleton> const obj);

    protected:
        task<GIOPDecoder*> _twowayCall(Stub *stub, const char *method, std::function<void(GIOPEncoder &)> encode);
};

}  // namespace CORBA
