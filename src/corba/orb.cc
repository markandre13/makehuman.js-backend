#include "orb.hh"

#include <format>
#include <iostream>
#include <map>
#include <print>
#include <stdexcept>
#include <string>

#include "cdr.hh"
#include "corba.hh"
#include "giop.hh"
#include "hexdump.hh"
#include "url.hh"
#include "ws/EventHandler.hh"

void sendChordata(void *data, size_t size);

using namespace std;

namespace CORBA {

class NamingContextExtImpl : public Skeleton {
        std::map<std::string, std::shared_ptr<ObjectBase>> name2Object;

    public:
        NamingContextExtImpl(CORBA::ORB *orb) : Skeleton(orb) {}
        const char *_idlClassName() const override;

        void bind(const std::string &name, std::shared_ptr<ObjectBase> servant) {
            if (name2Object.contains(name)) {
                throw runtime_error(format("name \"{}\" is already bound to object", name));
            }
            name2Object[name] = servant;
        }

        ObjectBase *resolve(const std::string &name) {
            println("NamingContextImpl.resolve(\"{}\")", name);
            auto servant = name2Object.find(name);
            if (servant == name2Object.end()) {
                println("NamingContextExtImpl::resolve(\"{}\"): name is not bound to an object", name);
                hexdump(name);
                throw runtime_error(format("NamingContextExtImpl::resolve(\"{}\"): name is not bound to an object", name));
            }
            return servant->second.get();
        }

    protected:
        void _orb_resolve(GIOPDecoder &decoder, GIOPEncoder &encoder) {
            auto entries = decoder.buffer.ulong();
            auto name = decoder.buffer.string();
            auto key = decoder.buffer.string();
            if (entries != 1 && !key.empty()) {
                cerr << "warning: resolve got " << entries << " (expected 1) and/or key is \"" << key << "\" (expected \"\")" << endl;
            }
            println("NamingContextExtImpl::_orb_resolve(): entries={}, name=\"{}\", key=\"{}\"", entries, name, key);
            auto result = resolve(std::string(name));  // FIXME: we don't want to copy the string
            encoder.object(result);
        }

        void _orb_resolve_str(GIOPDecoder &decoder, GIOPEncoder &encoder) {
            auto name = decoder.buffer.string();
            cerr << "NamingContextExtImpl::_orb_resolve_str(\"" << name << "\")" << endl;
            auto result = resolve(std::string(name));
            encoder.object(result);
        }

        void _call(const std::string_view &operation, GIOPDecoder &decoder, GIOPEncoder &encoder) override {
            // cerr << "NamingContextExtImpl::_call(" << operation << ", ...)"
            // << endl;
            if (operation == "resolve") {
                _orb_resolve(decoder, encoder);
                return;
            }
            if (operation == "resolve_str") {
                _orb_resolve_str(decoder, encoder);
                return;
            }
            // TODO: throw a BAD_OPERATION system exception here
            throw runtime_error(std::format("bad operation: '{}' does not exist", operation));
        }
};

const char *NamingContextExtImpl::_idlClassName() const { return "omg.org/CosNaming/NamingContextExt"; }

class NamingContextExtStub : public Stub {
    public:
        NamingContextExtStub(CORBA::ORB *orb, const std::string &remoteId, detail::Connection *connection) : Stub(orb, remoteId, connection) {}
        // static _idlClassName(): string {
        //     return "omg.org/CosNaming/NamingContextExt"
        // }

        // static narrow(object: any): NamingContextExtStub {
        //     if (object instanceof NamingContextExtStub)
        //         return object as NamingContextExtStub
        //     throw Error("NamingContextExt.narrow() failed")
        // }

        // TODO: the argument doesn't match the one in the IDL but for now it's
        // good enough
        task<unique_ptr<ObjectReference>> resolve_str(const string &name) {
            return orb->twowayCall<unique_ptr<ObjectReference>>(
                this, "resolve_str",
                [name](GIOPEncoder &encoder) {
                    encoder.string(name);
                },
                [](GIOPDecoder &decoder) {
                    return decoder.reference();
                });
        }
};

ObjectBase::~ObjectBase() {}

const char *ObjectBase::_idlClassName() const { return nullptr; }

bool ObjectBase::_is_a_remote(const char *repoid) { return true; }

ORB::ORB() {}

task<int> ORB::stringToObject(const std::string &iorString) {
    std::println("ORB::stringToObject(...): enter", iorString);
    auto uri = decodeURI(iorString);
    if (std::holds_alternative<IOR>(uri)) {
        // return iorToObject(std::get<IOR>(uri));
    }
    if (std::holds_alternative<CorbaName>(uri)) {
        auto name = std::get<CorbaName>(uri);
        auto addr = name.addr[0];  // TODO: we only try the 1st one
        if (addr.proto == "iiop") {
            // get remote NameService (FIXME: what if it's us?)
            std::println("ORB::stringToObject(...): get connection to {}:{}", addr.host, addr.port);
            CORBA::detail::Connection *nameConnection = co_await getConnection(addr.host, addr.port);
            std::println("ORB::stringToObject(...): got connection to {}:{}", addr.host, addr.port);
            auto it = nameConnection->stubsById.find(name.objectKey);
            NamingContextExtStub *rootNamingContext;
            if (it == nameConnection->stubsById.end()) {
                std::println("ORB::stringToObject(...): creating NamingContextExtStub");
                rootNamingContext = new NamingContextExtStub(this, name.objectKey, nameConnection);
                nameConnection->stubsById[name.objectKey] = rootNamingContext;
            } else {
                std::println("ORB::stringToObject(...): reusing NamingContextExtStub");
                rootNamingContext = dynamic_cast<NamingContextExtStub *>(it->second);
                if (rootNamingContext == nullptr) {
                    throw runtime_error("Not a NamingContextExt");
                }
            }
            // get object from remote NameServiceExt
            std::println("ORB::stringToObject(...): calling resolve_str() on remote end");
            auto reference = co_await rootNamingContext->resolve_str(name.name);

            // create stub for remote object
            // const objectConnection = await this.getConnection(reference.host,
            // reference.port); let object =
            // objectConnection.stubsById.get(reference.objectKey); if (object
            // !== undefined)
            //     return object;
            // const shortName = reference.oid.substring(4, reference.oid.length
            // - 4); let aStubClass =
            // objectConnection.orb.stubsByName.get(shortName); if (aStubClass
            // === undefined) {
            //     throw Error(`ORB: no stub registered for OID
            //     '${reference.oid} (${shortName})'`);
            // }
            // object = new aStubClass(objectConnection.orb,
            // reference.objectKey, objectConnection);
            // objectConnection.stubsById.set(reference.objectKey, object!);
            // return object;
        }
    }
    co_return 42;
}

task<detail::Connection *> ORB::getConnection(string host, uint16_t port) {
    println("getConnection [1]");
    if (host == "::1") {
        host = "localhost";
    }
    for (auto conn : connections) {
        if (conn->remoteAddress() == host && conn->remotePort() == port) {
            co_return conn;
        }
    }
    println("getConnection [2]");
    for (auto proto : protocols) {
        println("getConnection [3]");
        if (debug) {
            if (connections.size() == 0) {
                println("ORB : Creating new connection to {}:{} as no others exist", host, port);
            } else {
                println("ORB : Creating new connection to {}:{}, as none found to", host, port);
            }
            for (auto conn : connections) {
                println("ORB : active connection {}:{}", conn->remoteAddress(), conn->remotePort());
            }
        }
        println("getConnection [4]");
        CORBA::detail::Connection *connection = co_await proto->connect(this, host, port);
        co_return connection;
    }
    println("getConnection [X]");
    throw runtime_error(format("failed to allocate connection to {}:{}", host, port));
}

task<unique_ptr<CDRDecoder>> ORB::_twowayCall(Stub *stub, const char *method, std::function<void(GIOPEncoder &)> encode) {
    println("ORB::_twowayCall(stub, \"{}\", ...)", method);
    auto requestId = stub->connection->requestId;
    stub->connection->requestId += 2;
    GIOPEncoder encoder(stub->connection);
    auto responseExpected = true;
    encoder.encodeRequest(stub->objectId, method, requestId, responseExpected);
    encode(encoder);
    encoder.setGIOPHeader(GIOP_REQUEST);
    stub->connection->send((void *)encoder.buffer.data(), encoder.buffer.offset);

    println("BANG!!!");
    throw runtime_error("BANG");

    // return connection.interlock.suspend(requestId);
}

string ORB::registerServant(Skeleton *servant) {
    cerr << "ORB::registerServant(): WHO???" << endl;
    CDREncoder encoder;
    encoder.ulonglong(++servantIdCounter);
    string oid(encoder.data(), encoder.length());
    servants[oid] = servant;
    return oid;
}

void ORB::bind(const std::string &id, std::shared_ptr<CORBA::Skeleton> const obj) {
    if (namingService == nullptr) {
        println("ORB::bind(\"{}\"): CREATING NameService", id);
        namingService = new NamingContextExtImpl(this);
        servants["NameService"] = namingService;
    }
    namingService->bind(id, obj);
}

void ORB::_socketRcvd(const uint8_t *buffer, size_t size) {
    cout << "RECEIVED" << endl;
    hexdump(buffer, size);
    CORBA::CDRDecoder data((const char *)buffer, size);
    CORBA::GIOPDecoder decoder(data);
    auto type = decoder.scanGIOPHeader();
    switch (type) {
        case CORBA::GIOP_REQUEST: {
            auto request = decoder.scanRequestHeader();
            string objectKey = request->objectKey;  // FIXME: do not copy FIXME: length HACK
            cout << "REQUEST(requestId=" << request->requestId << ", objectKey='" << hex << objectKey << "', " << request->method << ")" << endl;
            auto servant = servants.find(objectKey);  // FIXME: avoid string copy
            if (servant == servants.end()) {
                cerr << "DIDN'T FIND KEY. CURRENTLY " << servants.size() << " SERVANTS REGISTERED" << endl;
                cerr << "LOOKING FOR" << endl;
                hexdump((unsigned char*)objectKey.data(), objectKey.size());
                cerr << "LOOKING HAVE" << endl;
                for(auto &s: servants) {
                    cerr << "  '" << s.first << "' ? " << (s.first == objectKey) << endl;
                    hexdump((unsigned char*)s.first.data(), s.first.size());
                }
                if (request->responseExpected) {
                    // send error message
                }
                cerr << "IDL:omg.org/CORBA/OBJECT_NOT_EXIST:1.0" << endl;
                return;
            }

            if (request->method == "_is_a") {
                return;
            }

            // auto method = servant._map.find(request->method);
            // if (method == servant._map.end()) {
            //     if (request->responseExpected) {
            //         // send error message
            //     }
            //     return;
            // }

            try {
                CORBA::GIOPEncoder encoder;
                encoder.skipReplyHeader();
                // std::cerr << "CALL" << std::endl;
                servant->second->_call(request->method, decoder, encoder);
                // std::cerr << "CALLED" << std::endl;
                if (request->responseExpected) {
                    auto length = encoder.buffer.offset;  // FIXME: buffer.offset and
                                                          // buffer.lenght() differ
                    encoder.setGIOPHeader(GIOP_REPLY);
                    encoder.setReplyHeader(request->requestId, GIOP_NO_EXCEPTION);
                    cout << "TODO: SEND " << hex << length << " OCTETS" << endl;
                    sendChordata((void *)encoder.buffer.data(), length);
                    // send reply
                }
            } catch (std::out_of_range e) {
                if (request->responseExpected) {
                    // send reply
                }
                cerr << "OUT OF RANGE: " << e.what() << endl;
            } catch (std::exception e) {
                if (request->responseExpected) {
                    // send reply
                }
                cerr << "EXCEPTION: " << e.what() << endl;
            }
        } break;
        default:
            cout << "GOT YET UNIMPLEMENTED REQUEST " << type << endl;
            break;
    }
}

}  // namespace CORBA
