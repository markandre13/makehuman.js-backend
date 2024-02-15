#include "corba.hh"
#include "orb.hh"

namespace CORBA {

Skeleton::Skeleton(ORB *orb) : ObjectBase(orb, orb->registerServant(this)) {}

};  // namespace CORBA
