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
#include "protocol.hh"
#include "url.hh"

using namespace std;

namespace CORBA {

class NamingContextExtImpl : public Skeleton {
        std::map<std::string, std::shared_ptr<Object>> name2Object;

    public:
        NamingContextExtImpl(CORBA::ORB *orb, const string &objectKey) : Skeleton(orb, objectKey) {}
        const char *repository_id() const override;

        void bind(const std::string &name, std::shared_ptr<Object> servant) {
            if (name2Object.contains(name)) {
                throw runtime_error(format("name \"{}\" is already bound to object", name));
            }
            name2Object[name] = servant;
        }

        std::shared_ptr<Object> resolve(const std::string &name) {
            println("NamingContextImpl.resolve(\"{}\")", name);
            auto servant = name2Object.find(name);
            if (servant == name2Object.end()) {
                println("NamingContextExtImpl::resolve(\"{}\"): name is not bound to an object", name);
                hexdump(name);
                throw runtime_error(format("NamingContextExtImpl::resolve(\"{}\"): name is not bound to an object", name));
            }
            println("NamingContextExtImpl::resolve(\"{}\"): found the object", name);
            return servant->second;
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
            std::shared_ptr<Object> result = resolve(name);  // FIXME: we don't want to copy the string
            println("NamingContextExtImpl::_orb_resolve(): resolved object, now encode to giop");
            encoder.object(result.get());
            println("NamingContextExtImpl::_orb_resolve(): encoded to giop, return");
        }

        void _orb_resolve_str(GIOPDecoder &decoder, GIOPEncoder &encoder) {
            auto name = decoder.buffer.string();
            cerr << "NamingContextExtImpl::_orb_resolve_str(\"" << name << "\")" << endl;
            auto result = resolve(name);
            encoder.object(result.get());
        }

        CORBA::task<> _call(const std::string &operation, GIOPDecoder &decoder, GIOPEncoder &encoder) override {
            // cerr << "NamingContextExtImpl::_call(" << operation << ", ...)"
            // << endl;
            if (operation == "resolve") {
                _orb_resolve(decoder, encoder);
                co_return;
            }
            if (operation == "resolve_str") {
                _orb_resolve_str(decoder, encoder);
                co_return;
            }
            // TODO: throw a BAD_OPERATION system exception here
            throw runtime_error(std::format("bad operation: '{}' does not exist", operation));
        }
};

const char *NamingContextExtImpl::repository_id() const { return "omg.org/CosNaming/NamingContextExt"; }

class NamingContextExtStub : public Stub {
    public:
        NamingContextExtStub(CORBA::ORB *orb, const std::string &objectKey, detail::Connection *connection) : Stub(orb, objectKey, connection) {}
        const char *repository_id() const override;

        // static narrow(object: any): NamingContextExtStub {
        //     if (object instanceof NamingContextExtStub)
        //         return object as NamingContextExtStub
        //     throw Error("NamingContextExt.narrow() failed")
        // }

        // TODO: the argument doesn't match the one in the IDL but for now it's
        // good enough
        task<shared_ptr<ObjectReference>> resolve_str(const string &name) {
            return get_ORB()->twowayCall<shared_ptr<ObjectReference>>(
                this, "resolve_str",
                [name](GIOPEncoder &encoder) {
                    encoder.string(name);
                },
                [](GIOPDecoder &decoder) {
                    return decoder.reference();
                });
        }
};

const char *NamingContextExtStub::repository_id() const { return "omg.org/CosNaming/NamingContextExt"; }

Object::~Object() {}

ORB::ORB() {}

task<shared_ptr<Object>> ORB::stringToObject(const std::string &iorString) {
    std::println("ORB::stringToObject(\"{}\"): enter", iorString);
    auto uri = decodeURI(iorString);
    if (std::holds_alternative<IOR>(uri)) {
        // return iorToObject(std::get<IOR>(uri));
    }
    if (std::holds_alternative<CorbaName>(uri)) {
        auto name = std::get<CorbaName>(uri);
        auto addr = name.addr[0];  // TODO: we only try the 1st one
        if (addr.proto == "iiop") {
            // get remote NameService (FIXME: what if it's us?)
            std::println("ORB::stringToObject(\"{}\"): get connection to {}:{}", iorString, addr.host, addr.port);
            CORBA::detail::Connection *nameConnection = getConnection(addr.host, addr.port);
            std::println("ORB::stringToObject(\"{}\"): got connection to {}:{}", iorString, addr.host, addr.port);
            auto it = nameConnection->stubsById.find(name.objectKey);
            NamingContextExtStub *rootNamingContext;
            if (it == nameConnection->stubsById.end()) {
                std::println("ORB::stringToObject(\"{}\"): creating NamingContextExtStub(orb, objectKey=\"{}\", connection)", iorString, name.objectKey);
                rootNamingContext = new NamingContextExtStub(this, name.objectKey, nameConnection);
                nameConnection->stubsById[name.objectKey] = rootNamingContext;
            } else {
                std::println("ORB::stringToObject(\"{}\"): reusing NamingContextExtStub", iorString);
                rootNamingContext = dynamic_cast<NamingContextExtStub *>(it->second);
                if (rootNamingContext == nullptr) {
                    throw runtime_error("Not a NamingContextExt");
                }
            }
            // get object from remote NameServiceExt
            std::println("ORB::stringToObject(\"{}\"): calling resolve_str(\"{}\") on remote end", iorString, name.name);
            auto reference = co_await rootNamingContext->resolve_str(name.name);
            std::println("ORB::stringToObject(\"{}\"): got reference", iorString);
            reference->setORB(this);
            co_return dynamic_pointer_cast<Object, ObjectReference>(reference);
        }
    }
    throw runtime_error(format("ORB::stringToObject(\"{}\") failed", iorString));
}

detail::Connection *ORB::getConnection(string host, uint16_t port) {
    if (host == "::1") {
        host = "localhost";
    }
    for (auto conn : connections) {
        if (conn->remoteAddress() == host && conn->remotePort() == port) {
            return conn;
        }
    }
    for (auto proto : protocols) {
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
        CORBA::detail::Connection *connection = proto->connect(this, host, port);
        connections.push_back(connection);
        return connection;
    }
    throw runtime_error(format("failed to allocate connection to {}:{}", host, port));
}

task<GIOPDecoder *> ORB::_twowayCall(Stub *stub, const char *operation, std::function<void(GIOPEncoder &)> encode) {
    println("ORB::_twowayCall(stub, \"{}\", ...) ENTER", operation);
    if (stub->connection == nullptr) {
        throw runtime_error("ORB::_twowayCall(): the stub has no connection");
    }
    auto requestId = stub->connection->requestId;
    stub->connection->requestId += 2;
    printf("CONNECTION %p %s:%u -> %s:%u requestId=%u\n", static_cast<void *>(stub->connection), stub->connection->localAddress().c_str(),
           stub->connection->localPort(), stub->connection->remoteAddress().c_str(), stub->connection->remotePort(), stub->connection->requestId);
    GIOPEncoder encoder(stub->connection);
    auto responseExpected = true;
    encoder.encodeRequest(stub->objectKey, operation, requestId, responseExpected);
    encode(encoder);
    encoder.setGIOPHeader(GIOP_REQUEST);
    println("ORB::_twowayCall(stub, \"{}\", ...) SEND REQUEST objectKey=\"{}\", operation=\"{}\", requestId={}", operation, stub->objectKey, operation,
            requestId);
    stub->connection->send((void *)encoder.buffer.data(), encoder.buffer.offset);
    println("ORB::_twowayCall(stub, \"{}\", ...) WAIT FOR REPLY", operation);
    GIOPDecoder *decoder = co_await stub->connection->interlock.suspend(requestId);
    println("ORB::_twowayCall(stub, \"{}\", ...) LEAVE WITH DECODER", operation);

    // move parts of this into a separate function so that it can be unit tested
    switch (decoder->replyStatus) {
        case GIOP_NO_EXCEPTION:
            break;
        case GIOP_USER_EXCEPTION:
            throw UserException();
            // throw runtime_error(format("CORBA User Exception from {}:{}", stub->connection->remoteAddress(), stub->connection->remotePort()));
            break;
        case GIOP_SYSTEM_EXCEPTION: {
            // 0.4.3.2 ReplyBody: SystemExceptionReplyBody
            auto exceptionId = decoder->string();
            auto minorCodeValue = decoder->ulong();
            auto completionStatus = static_cast<CompletionStatus>(decoder->ulong());
            if (exceptionId == "IDL:omg.org/CORBA/MARSHAL:1.0") {
                throw MARSHAL(minorCodeValue, completionStatus);
            } else if (exceptionId == "IDL:omg.org/CORBA/NO_PERMISSION:1.0") {
                throw NO_PERMISSION(minorCodeValue, completionStatus);
            } else if (exceptionId == "IDL:omg.org/CORBA/BAD_PARAM:1.0") {
                throw BAD_PARAM(minorCodeValue, completionStatus);
            } else if (exceptionId == "IDL:omg.org/CORBA/BAD_OPERATION:1.0") {
                throw BAD_OPERATION(minorCodeValue, completionStatus);
            } else if (exceptionId == "IDL:omg.org/CORBA/OBJECT_NOT_EXIST:1.0") {
                throw OBJECT_NOT_EXIST(minorCodeValue, completionStatus);
            } else if (exceptionId == "IDL:omg.org/CORBA/TRANSIENT:1.0") {
                throw TRANSIENT(minorCodeValue, completionStatus);
            } else if (exceptionId == "IDL:omg.org/CORBA/OBJECT_ADAPTER:1.0") {
                throw OBJECT_ADAPTER(minorCodeValue, completionStatus);
            } else if (exceptionId == "IDL:mark13.org/CORBA/GENERIC:1.0") {
                throw runtime_error(
                    format("Remote CORBA exception from {}:{}: {}", stub->connection->remoteAddress(), stub->connection->remotePort(), decoder->string()));
            } else {
                throw runtime_error(
                    format("CORBA System Exception {} from {}:{}", exceptionId, stub->connection->remoteAddress(), stub->connection->remotePort()));
            }
        } break;
        default:
            throw runtime_error(format("ReplyStatusType {} is not supported", (unsigned)decoder->replyStatus));
    }

    co_return decoder;
}

void ORB::onewayCall(Stub *stub, const char *operation, std::function<void(GIOPEncoder &)> encode) {
    if (stub->connection == nullptr) {
        throw runtime_error("ORB::onewayCall(): the stub has no connection");
    }
    auto requestId = stub->connection->requestId;
    stub->connection->requestId += 2;
    GIOPEncoder encoder(stub->connection);
    auto responseExpected = true;
    encoder.encodeRequest(stub->objectKey, operation, requestId, responseExpected);
    encode(encoder);
    encoder.setGIOPHeader(GIOP_REQUEST);
    stub->connection->send((void *)encoder.buffer.data(), encoder.buffer.offset);
}

string ORB::registerServant(Skeleton *servant) {
    println("ORB::registerServant(servant)");
    string objectKey = format("OID:{:x}", ++servantIdCounter);
    servants[objectKey] = servant;
    return objectKey;
}

string ORB::registerServant(Skeleton *servant, const string &objectKey) {
    println("ORB::registerServant(servant, \"{}\")", objectKey);
    servants[objectKey] = servant;
    return objectKey;
}

void ORB::bind(const std::string &id, std::shared_ptr<CORBA::Skeleton> const obj) {
    if (namingService == nullptr) {
        println("ORB::bind(\"{}\"): CREATING NameService", id);
        namingService = new NamingContextExtImpl(this, "NameService");
        servants["NameService"] = namingService;
    }
    namingService->bind(id, obj);
}

CORBA::task<> ORB::_socketRcvd(detail::Connection *connection, const uint8_t *buffer, size_t size) {
    cout << "RECEIVED" << endl;
    hexdump(buffer, size);

    CORBA::CDRDecoder data((const char *)buffer, size);
    CORBA::GIOPDecoder decoder(data);
    auto type = decoder.scanGIOPHeader();
    switch (type) {
        case CORBA::GIOP_REQUEST: {
            // TODO: move this into a method
            auto request = decoder.scanRequestHeader();
            string objectKey = request->objectKey;  // FIXME: do not copy FIXME: length HACK
            cout << "REQUEST(requestId=" << request->requestId << ", objectKey='" << hex << objectKey << "', " << request->method << ")" << endl;
            auto servant = servants.find(objectKey);  // FIXME: avoid string copy
            if (servant == servants.end()) {
                if (request->responseExpected) {
                    CORBA::GIOPEncoder encoder(connection);
                    encoder.skipReplyHeader();

                    encoder.string("IDL:omg.org/CORBA/OBJECT_NOT_EXIST:1.0");
                    encoder.ulong(0x4f4d0001);  // Attempt to pass an unactivated (unregistered) value as an object reference.
                    encoder.ulong(NO);          // completionStatus

                    auto length = encoder.buffer.offset;
                    encoder.setGIOPHeader(GIOP_REPLY);
                    encoder.setReplyHeader(request->requestId, GIOP_SYSTEM_EXCEPTION);

                    connection->send((void *)encoder.buffer.data(), length);
                }
                co_return;
            }

            if (request->method == "_is_a") {
                auto repositoryId = decoder.string();
                CORBA::GIOPEncoder encoder(connection);
                encoder.skipReplyHeader();

                encoder.boolean(repositoryId == servant->second->repository_id());

                auto length = encoder.buffer.offset;
                encoder.setGIOPHeader(GIOP_REPLY);
                encoder.setReplyHeader(request->requestId, GIOP_NO_EXCEPTION);
                connection->send((void *)encoder.buffer.data(), length);

                co_return;
            }

            try {
                CORBA::GIOPEncoder encoder(connection);
                encoder.skipReplyHeader();
                // move parts of this into a separate function so that it can be unit tested
                std::cerr << "CALL SERVANT" << std::endl;
                try {
                    co_await servant->second->_call(request->method, decoder, encoder);
                } catch (CORBA::UserException &ex) {
                    if (request->responseExpected) {
                        auto length = encoder.buffer.offset;
                        encoder.setGIOPHeader(GIOP_REPLY);
                        encoder.setReplyHeader(request->requestId, GIOP_USER_EXCEPTION);
                        connection->send((void *)encoder.buffer.data(), length);
                    } break;
                } catch (CORBA::SystemException &error) {
                    println("SERVANT THREW SYSTEM EXCEPTION");
                    if (request->responseExpected) {
                        encoder.string(error.major());
                        encoder.ulong(error.minor);
                        encoder.ulong(error.completed);
                        auto length = encoder.buffer.offset;
                        encoder.setGIOPHeader(GIOP_REPLY);
                        encoder.setReplyHeader(request->requestId, GIOP_SYSTEM_EXCEPTION);
                        connection->send((void *)encoder.buffer.data(), length);
                    }
                    break;
                } catch (std::exception &ex) {
                    if (request->responseExpected) {
                        encoder.string("IDL:mark13.org/CORBA/GENERIC:1.0");
                        encoder.ulong(0);
                        encoder.ulong(0);
                        encoder.string(format("IDL:{}:1.0: {}", typeid(ex).name(), ex.what()));
                        auto length = encoder.buffer.offset;
                        encoder.setGIOPHeader(GIOP_REPLY);
                        encoder.setReplyHeader(request->requestId, GIOP_SYSTEM_EXCEPTION);
                        connection->send((void *)encoder.buffer.data(), length);
                    }
                    break;
                } catch (...) {
                    println("SERVANT THREW EXCEPTION");
                }
                std::cerr << "CALLED SERVANT" << std::endl;
                if (request->responseExpected) {
                    auto length = encoder.buffer.offset;
                    encoder.setGIOPHeader(GIOP_REPLY);
                    encoder.setReplyHeader(request->requestId, GIOP_NO_EXCEPTION);
                    println("ORB::_socketRcvd(): send REPLY via connection->send(...)");
                    connection->send((void *)encoder.buffer.data(), length);
                }
            } catch (std::out_of_range &e) {
                if (request->responseExpected) {
                    // send reply
                }
                cerr << "OUT OF RANGE: " << e.what() << endl;
            } catch (std::exception &e) {
                if (request->responseExpected) {
                    // send reply
                }
                cerr << "ORB::_socketRcvd: EXCEPTION: " << e.what() << endl;
            }
        } break;
        case CORBA::GIOP_REPLY: {
            auto _data = decoder.scanReplyHeader();
            if (!connection->interlock.resume(_data->requestId, &decoder)) {
                println("ORB::_socketRcvd(): unexpected reply to requestId {}", _data->requestId);
                connection->interlock.print();
            }
            break;
        } break;
        default:
            cout << "ORB::_socketRcvd(): GOT YET UNIMPLEMENTED REQUEST " << type << endl;
            break;
    }
    co_return;
}

}  // namespace CORBA
