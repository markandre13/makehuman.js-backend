#pragma once

#include "makehuman_skel.hh"

class Backend_impl : public Backend_skel {
        std::shared_ptr<Frontend> frontend;
    public:
        Backend_impl(std::shared_ptr<CORBA::ORB> orb);
        CORBA::async<> setFrontend(std::shared_ptr<Frontend> frontend) override;
};

// this would only be needed for testing

// class Frontend_impl : public Frontend_skel {
//     public:
//         Frontend_impl(std::shared_ptr<CORBA::ORB> orb);
//         CORBA::async<void> hello() override;
// };
