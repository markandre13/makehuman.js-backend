#pragma once

#include "makehuman_skel.hh"

class Backend_impl : public Backend_skel {
    public:
        Backend_impl(std::shared_ptr<CORBA::ORB> orb);
        virtual CORBA::async<std::string> hello(const std::string_view &word) override;
        virtual CORBA::async<> fail() override;
};
