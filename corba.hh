#pragma once

#include <string>
#include <memory>

namespace CORBA {
    class Skeleton;

    class ORB {
        public:
            void run();

            //
            // NameService
            //
            void bind(const std::string &id, std::shared_ptr<CORBA::Skeleton> const &obj);
    };

    class Object {
        protected:
            std::shared_ptr<ORB> orb;
            Object(const std::shared_ptr<ORB> &orb): orb(orb) { }
    };

    class Skeleton: Object {
        protected:
            Skeleton(const std::shared_ptr<CORBA::ORB> &orb): Object(orb) { }
    };
}