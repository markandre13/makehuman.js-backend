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
    Backend_skel(std::shared_ptr<CORBA::ORB> orb) : Skeleton(orb) {}
private:
    CORBA::async<> _call(const std::string_view &operation, CORBA::GIOPDecoder &decoder, CORBA::GIOPEncoder &encoder) override;
};

class Backend2_skel: public CORBA::Skeleton, public Backend2 {
public:
    Backend2_skel(std::shared_ptr<CORBA::ORB> orb) : Skeleton(orb) {}
private:
    CORBA::async<> _call(const std::string_view &operation, CORBA::GIOPDecoder &decoder, CORBA::GIOPEncoder &encoder) override;
};

