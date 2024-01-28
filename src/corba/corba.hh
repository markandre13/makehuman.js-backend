#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>
#include <functional>

#include "giop.hh"

namespace CORBA {

class ObjectBase;
class Stub;
class Skeleton;
class NamingContextExtImpl;

class ORB : public std::enable_shared_from_this<ORB> {
        std::shared_ptr<NamingContextExtImpl> namingService;
        // std::map<std::string, Skeleton*> initialReferences; // name to
        std::map<std::string, Skeleton *> servants;  // objectId to skeleton

        uint64_t servantIdCounter = 0;

    public:
        ORB();
        void run();

        static void socketRcvd(const uint8_t *buffer, size_t size);
        void _socketRcvd(const uint8_t *buffer, size_t size);

        std::string registerServant(Skeleton *skeleton);
        template<typename T> T
        twowayCall(
            Stub *stub,
            const char *method,
            std::function<void(GIOPEncoder&)> encode,
            std::function<T(GIOPDecoder&)> decode
        ) {
            auto cdr = _twowayCall(stub, method, encode);
            GIOPDecoder decoder(*cdr);
            auto result = decode(decoder);
            return result;
        }

        //
        // NameService
        //
        void bind(const std::string &id, std::shared_ptr<CORBA::Skeleton> const obj);
    protected:
        std::unique_ptr<CDRDecoder> _twowayCall(
            Stub *stub,
            const char *method,
            std::function<void(GIOPEncoder&)> encode
        );
};

class ObjectBase {
    protected:
        std::shared_ptr<ORB> orb;

    public:
        ObjectBase(std::shared_ptr<ORB> orb) : orb(orb) {}
        ObjectBase(std::shared_ptr<ORB> orb, std::string id) : orb(orb), id(id) {}
        std::string id;                             // objectId
        virtual const char *_idlClassName() const;  // TODO: rename into _repoid() & 
        virtual bool _is_a_remote(const char *repoid);
};

/**
 * An object reference.
 */
class Object : public CORBA::ObjectBase {
    public:
        Object(std::shared_ptr<CORBA::ORB> orb) : ObjectBase(orb) {}
        // ORB
        // Connection
        // bool _narrow_helper(const char *id);
        // const char * _repoid();
        // bool _is_a_remote(const char *id);
};

/**
 * Base class for representing remote objects.
 */
class Stub : public ObjectBase {
    public:
        Stub(std::shared_ptr<CORBA::ORB> orb) : ObjectBase(orb) {}
};

/**
 * Base class for representing local objects.
 */
class Skeleton : public ObjectBase {
    public:
        Skeleton(std::shared_ptr<CORBA::ORB> orb) : ObjectBase(orb, orb->registerServant(this)) {}
        virtual void _call(const std::string_view &operation, GIOPDecoder &decoder, GIOPEncoder &encoder) = 0;
};

}  // namespace CORBA