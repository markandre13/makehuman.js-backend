import std;

// 8.3 Object Reference Operations; CORBA 3.3 Part 1 Interfaces
//
// interface Object { // pseudo idl
//   InterfaceDef get_interface();
//   boolean is_nil();
//   Object duplicate();
//   void release();
//   boolean is_a(in RepositoryId logical_type_id);
//   boolean non_existent();
//   boolean is_equivalent(in Object other_object);
//   unsigned long hash(in unsigned long);
//   void create_request(in Context ctx,
//                       in Identifier operation,
//                       in NVList arg_list,
//                       inout NamedValue result,
//                       out Request req,
//                       in Flags req_flag);
//   ...
//   Policy get_policy (in PolicyType policy_type);
//   Object get_component();
//   string repository_id();
//   ORB get_orb(); // when called on local object, throws NO_IMPLEMENT with standard minor code 8
// };
//
// interface LocalObject: Object {
//
// }
//
// Orbix: all user-defined interfaces inherit Object
// Object
//   const string_view& repository_id() const;
// Stub
//   (orb: where to send to)
//   connection: (would be enough if orb is stored in connection?)
//   oid
// Skeleton
//   does it need an orb? could be used by many orbs
//   does it need an oid? could be different per orb
//   the orb stores it in it's servant list
// Reference

// interface A { };
// class A
// {
//   public:
//        static A_ptr _duplicate(A_ptr obj);
//        static A_ptr _narrow(A_ptr obj);
// };

// const IOP::IOR & ior() const
// boolean is_local() const
// Object(stub, coolocated = false, servar, cor, ...)
// ORB_ptr _get_orb()

using namespace std;

class Object {
    public:
        virtual ~Object();
        virtual const std::string_view& repository_id() = 0;
};

class Stub : virtual public Object {};

class Skeleton : virtual public Object {};

class Backend : virtual public Object {
        static string_view _rid;

    public:
        virtual const std::string_view& repository_id() override;
};

Object::~Object() {}
string_view Backend::_rid("IDL:Backend:1.0");
const std::string_view& Backend::repository_id() { return _rid; }

class Backend_stub : public Stub, public Backend {};

class Backend_skel : public Skeleton, public Backend {};

class Backend_impl : public Backend_skel {};

int main() {
    println("RUN");
    auto impl = new Backend_impl();
    auto stub = new Backend_stub();

    Object *b = impl;
    if (dynamic_cast<Skeleton*>(b)) {
        println("impl is a skeleton");
    }

    return 0;
}