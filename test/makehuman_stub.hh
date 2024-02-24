// This file is generated by the corba.js IDL compiler from '/Users/mark/c/mediapipe/test/makehuman.idl'.

#pragma once
#include "../src/corba/corba.hh"
#include "../src/corba/orb.hh"
#include "../src/corba/giop.hh"
#include "../src/corba/coroutine.hh"
#include <vector>
#include "makehuman.hh"

class Backend_stub: public Backend, public CORBA::Stub {
public:
    Backend_stub(CORBA::ORB *orb, const std::string &objectKey, CORBA::detail::Connection *connection): Stub(orb, objectKey, connection) {}
    const char *repository_id() const override { return "IDL:Backend:1.0"; }
    virtual CORBA::async<std::string> hello(std::string hello) override;
    virtual CORBA::async<void> fail() override;
};

class Backend2_stub: public Backend2, public CORBA::Stub {
public:
    Backend2_stub(CORBA::ORB *orb, const std::string &objectKey, CORBA::detail::Connection *connection): Stub(orb, objectKey, connection) {}
    const char *repository_id() const override { return "IDL:Backend2:1.0"; }
    virtual void chordata(bool on) override;
    virtual void mediapipe(bool on) override;
};

