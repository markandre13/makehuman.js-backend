#pragma once

#include "object.hh"

namespace CORBA {

// TODO: rename this into IOR. 'CORBA 3.3 Pt 1, 6.2.4' already 'Object Reference' for 'Object Key'
class IOR : public Object {
        std::shared_ptr<CORBA::ORB> orb;

    public:
        std::string oid;   // object/repository id the type, e.g. IDL:MyClass:1.0
        std::string host;  // ORB host
        uint16_t port;     // ORB port
        blob objectKey;    // identifies the current object instance on the ORB

        IOR(const std::string &ior);
        IOR(std::shared_ptr<CORBA::ORB> orb, const std::string_view &oid, std::string host, uint16_t port, const CORBA::blob_view &objectKey)
            : orb(orb), oid(oid), host(host), port(port), objectKey(objectKey) {}
        virtual ~IOR() override;

        // this is a hack for _narrow(shared_ptr<Object>)
        inline void setORB(std::shared_ptr<CORBA::ORB> anORB) { orb = anORB; }

        virtual std::string_view repository_id() const override { return oid; }
        virtual blob_view get_object_key() const override { return objectKey; }
        std::shared_ptr<CORBA::ORB> get_ORB() const override { return orb; }
};

}  // namespace CORBA
