#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "coroutine.hh"
#include "giop.hh"

namespace CORBA {

namespace detail {
class Connection;
}

// okay, i'm not sure about how thy ol' CORBA vendors did it, here's what i could do:
// * when we call resolve_str(name) on the remove name server, we get a reference, which contains
//   host, port, oid, objectKey (oid == repositoryId == "IDL:Foo:1.0")
// * the same object could actually have different IORs to a remote peer, so it doesn't make
//   sense to store one in every object
// * _narrow could be called on
//   * remote references -> create a stub
//   * local skeleton implementations -> just return the object itself when the type matches
//   * remote stubs -> either create the same stub or another stub for the type?

// to convert an Object into a Stub with _narrow(), I need:...:
// * repository id, e.g. "IDL:Foobar:1.0", for _narrow() to check the Object is legit
// *

class Object {
    protected:
        CORBA::ORB *orb;

    public:
        Object(CORBA::ORB *orb, const std::string &objectKey) : orb(orb), objectKey(objectKey) {
            std::println("Object(orb, \"{}\")", this->objectKey);
        }
        virtual ~Object();

        // a unique id for this object on this ORB
        std::string objectKey;

        // ORB
        // Connection
        // bool _narrow_helper(const char *id);
        // const char * _repoid();
        // bool _is_a_remote(const char *id);

        // this is actually defined in CORBA_Object.idl

        virtual const char *repository_id() const {
            throw std::runtime_error("Object::repository_id() has been called. Maybe you did you call it from within a constructor?");
        }
        // bool is_nil();
        // virtual is_a(string logical_type_id)
        CORBA::ORB *get_ORB() { return orb; }
};

class ObjectReference : public Object {
    public:
        ObjectReference(CORBA::ORB *orb, std::string oid, std::string host, uint16_t port, std::string objectKey)
            : Object(orb, objectKey), oid(oid), host(host), port(port) {}
        virtual ~ObjectReference() override;
        void setORB(CORBA::ORB *anORB) {this->orb = anORB; }

        std::string oid;
        std::string host;
        uint16_t port;
        const char *repository_id() const override { return oid.c_str(); }
};

/**
 * Base class for representing remote objects.
 */
class Stub : public Object {
        friend class ORB;
        detail::Connection *connection;

    public:
        Stub(CORBA::ORB *orb, const std::string &objectKey, detail::Connection *connection) : Object(orb, objectKey), connection(connection) {}
        virtual ~Stub() override;
};

/**
 * Base class for representing local objects.
 */
class Skeleton : public Object {
    public:
        Skeleton(CORBA::ORB *orb);
        Skeleton(CORBA::ORB *orb, const std::string &objectKey);
        virtual ~Skeleton() override;
        virtual CORBA::async<> _call(const std::string &operation, GIOPDecoder &decoder, GIOPEncoder &encoder) = 0;
};

}  // namespace CORBA