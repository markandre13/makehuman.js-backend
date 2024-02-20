// This file is generated by the corba.js IDL compiler from '/Users/mark/c/mediapipe/test/makehuman.idl'.

#pragma once
#include "../src/corba/orb.hh"
#include "../src/corba/giop.hh"
#include "../src/corba/coroutine.hh"
#include <string>
#include <vector>

class Backend {
public:
    virtual CORBA::task<std::string> hello(std::string hello) = 0;
    virtual CORBA::task<void> fail() = 0;
    static std::shared_ptr<Backend> _narrow(std::shared_ptr<CORBA::Object> pointer);
};

class Backend2 {
public:
    virtual void chordata(bool on) = 0;
    virtual void mediapipe(bool on) = 0;
    static std::shared_ptr<Backend2> _narrow(std::shared_ptr<CORBA::Object> pointer);
};

