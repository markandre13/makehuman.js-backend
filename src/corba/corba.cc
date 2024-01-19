#include "corba.hh"
#include "ws/EventHandler.hh"

namespace CORBA {
    void ORB::run() {
        wsInit();
        wsHandle(true);
    }

    void ORB::bind(const std::string &id, std::shared_ptr<CORBA::Skeleton> const &obj) {
    }
}
