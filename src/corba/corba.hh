#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "exception.hh"
#include "coroutine.hh"
#include "giop.hh"

namespace CORBA {

namespace detail {
class Connection;
}

// CORBA 3.3 Part 1 Interfaces, 8.3 Object Reference Operations
class Object {
    public:
        virtual ~Object();
        virtual std::string_view repository_id() const = 0;
        virtual blob_view get_object_key() const = 0;
        virtual CORBA::ORB *get_ORB() const = 0;
};

// TODO: rename this into IOR. 'CORBA 3.3 Pt 1, 6.2.4' already 'Object Reference' for 'Object Key'
class ObjectReference : public Object {
        ORB *orb;

    public:
        std::string oid;   // object/repository id the type, e.g. IDL:MyClass:1.0
        std::string host;  // ORB host
        uint16_t port;     // ORB port
        blob objectKey;    // identifies the current object instance on the ORB

        ObjectReference(CORBA::ORB *orb, const std::string_view &oid, std::string host, uint16_t port, const CORBA::blob_view &objectKey)
            : orb(orb), oid(oid), host(host), port(port), objectKey(objectKey) {}
        virtual ~ObjectReference() override;

        // this is a hack for _narrow(shared_ptr<Object>)
        inline void setORB(CORBA::ORB *anORB) { this->orb = anORB; }

        virtual std::string_view repository_id() const override { return oid; }
        virtual blob_view get_object_key() const override { return objectKey; }
        CORBA::ORB *get_ORB() const override { return orb; }
};

/**
 * Base class for representing remote objects.
 */
class Stub : public virtual Object {
        friend class ORB;
        ORB *orb;
        blob objectKey; // objectKey used on the remote end of the connection
        detail::Connection *connection; // connection to where the remote object lives

    public:
        Stub(CORBA::ORB *orb, const CORBA::blob_view &objectKey, detail::Connection *connection) : orb(orb), objectKey(objectKey), connection(connection) {}
        virtual ~Stub() override;
        virtual blob_view get_object_key() const override { return objectKey; }
        CORBA::ORB *get_ORB() const override { return orb; }
};

/**
 * Base class for representing local objects.
 */
class Skeleton : public virtual Object {
    friend class ORB;
        ORB *orb;
        blob objectKey;
    public:
        Skeleton(CORBA::ORB *orb); // the ORB will create an objectKey
        Skeleton(CORBA::ORB *orb, const std::string &objectKey); // for special objectKeys, e.g. "omg.org/CosNaming/NamingContextExt"
        virtual ~Skeleton() override;
        virtual blob_view get_object_key() const override { return objectKey; }
        CORBA::ORB *get_ORB() const override { return orb; }
        virtual CORBA::async<> _call(const std::string_view &operation, GIOPDecoder &decoder, GIOPEncoder &encoder) = 0;
};

}  // namespace CORBA