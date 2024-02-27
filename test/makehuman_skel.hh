// This file is generated by the corba.js IDL compiler from 'makehuman.idl'.

#pragma once
#include <corba/corba.hh>
#include <corba/orb.hh>
#include <corba/giop.hh>
#include <corba/coroutine.hh>
#include <vector>
#include "makehuman.hh"

class Backend_skel: public CORBA::Skeleton, public Backend {
public:
    Backend_skel(CORBA::ORB *orb) : Skeleton(orb) {}
    const char *repository_id() const override { return "IDL:Backend:1.0"; }
private:
    CORBA::async<> _call(const std::string_view &operation, CORBA::GIOPDecoder &decoder, CORBA::GIOPEncoder &encoder) override;
};

class Backend2_skel: public CORBA::Skeleton, public Backend2 {
public:
    Backend2_skel(CORBA::ORB *orb) : Skeleton(orb) {}
    const char *repository_id() const override { return "IDL:Backend2:1.0"; }
private:
    CORBA::async<> _call(const std::string_view &operation, CORBA::GIOPDecoder &decoder, CORBA::GIOPEncoder &encoder) override;
};

