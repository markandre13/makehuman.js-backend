#include "corba.hh"
#include "orb.hh"

namespace CORBA {

Skeleton::Skeleton(CORBA::ORB *orb) : orb(orb), objectKey(orb->registerServant(this)) {}
Skeleton::Skeleton(CORBA::ORB *orb, const std::string &objectKey) : orb(orb), objectKey(objectKey) { orb->registerServant(this, objectKey); }

Skeleton::~Skeleton() {}
Stub::~Stub() {}
IOR::~IOR() {}

};  // namespace CORBA
