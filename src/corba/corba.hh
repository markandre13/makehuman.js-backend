#pragma once

#include <string>
#include <memory>
#include <vector>
#include <map>

namespace CORBA {

class Object;
class Skeleton;
class NamingContextExtImpl;
class GIOPDecoder;
class GIOPEncoder;

class ORB: public std::enable_shared_from_this<ORB> {
        std::shared_ptr<NamingContextExtImpl> namingService;
        // std::map<std::string, Skeleton*> initialReferences; // name to 
        std::map<std::string, Skeleton*> servants; // objectId to skeleton

    public:
        ORB();
        void run();

        static void socketRcvd(const uint8_t *buffer, size_t size);
        void _socketRcvd(const uint8_t *buffer, size_t size);

        //
        // NameService
        //
        void bind(const std::string &id, std::shared_ptr<CORBA::Skeleton> const obj);
};

class Object {
    protected:
        std::shared_ptr<ORB> orb;

    public:
        Object(std::shared_ptr<ORB> orb) : orb(orb) {}
        virtual ~Object();
        std::vector<uint8_t> id;
        virtual const char * _idlClassName() const;
};

class Stub : public Object {
    public:
        Stub(std::shared_ptr<CORBA::ORB> orb) : Object(orb) {}
};

class Skeleton : public Object {
    public:
        Skeleton(std::shared_ptr<CORBA::ORB> orb) : Object(orb) {}
        virtual void _call(const std::string_view &operation, GIOPDecoder &decoder, GIOPEncoder &encoder) = 0;
};

// Stub

}  // namespace CORBA