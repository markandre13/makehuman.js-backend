#include "corba.hh"
#include "orb.hh"

namespace CORBA {

Skeleton::Skeleton(CORBA::ORB *orb) : Object(orb, orb->registerServant(this)) {}
Skeleton::Skeleton(CORBA::ORB *orb, const std::string &objectKey) : Object(orb, objectKey) { orb->registerServant(this, objectKey); }

Skeleton::~Skeleton() {}
Stub::~Stub() {}
ObjectReference::~ObjectReference() {}

};  // namespace CORBA
