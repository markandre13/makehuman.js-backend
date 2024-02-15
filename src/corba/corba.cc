#include "orb.hh"
#include "corba.hh"

#include <format>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>

#include "cdr.hh"
#include "giop.hh"
#include "ws/EventHandler.hh"

using namespace std;

namespace CORBA {

static ORB *currentORB = nullptr;
 
void ORB::run() {
    currentORB = this;
    wsInit();
    while (true) {
        wsHandle(true);
    }
}

void ORB::socketRcvd(const uint8_t *buffer, size_t size) { currentORB->_socketRcvd(buffer, size); }

}  // namespace CORBA
