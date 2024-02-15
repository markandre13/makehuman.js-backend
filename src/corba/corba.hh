#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>
#include <functional>

#include "giop.hh"
#include "coroutine.hh"

namespace CORBA {

namespace detail {
    struct Connection;
}

class ObjectBase {
    protected:
        CORBA::ORB * orb;

    public:
        ObjectBase(CORBA::ORB * orb) : orb(orb) {}
        ObjectBase(CORBA::ORB * orb, std::string id) : orb(orb), id(id) {}
        virtual ~ObjectBase();
        std::string id;                             // objectId
        virtual const char *_idlClassName() const;  // TODO: rename into _repoid() & 
        virtual bool _is_a_remote(const char *repoid);
};

/**
 * An object reference.
 */
class Object : public CORBA::ObjectBase {
    public:
        Object(CORBA::ORB * orb) : ObjectBase(orb) {}
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
    friend class ORB;
    std::string objectId;
    detail::Connection *connection;
    public:
        Stub(CORBA::ORB * orb, const std::string &remoteId, detail::Connection *connection) : ObjectBase(orb), objectId(remoteId), connection(connection) {}
};

/**
 * Base class for representing local objects.
 */
class Skeleton : public ObjectBase {
    public:
        Skeleton(CORBA::ORB * orb);
        virtual void _call(const std::string_view &operation, GIOPDecoder &decoder, GIOPEncoder &encoder) = 0;
};

}  // namespace CORBA