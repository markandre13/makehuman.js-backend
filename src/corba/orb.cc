#include <format>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>

#include "cdr.hh"
#include "corba.hh"
#include "giop.hh"
#include "ws/EventHandler.hh"

#include "hexdump.hh"

void sendChordata(void* data, size_t size);

using namespace std;

namespace CORBA {

class NamingContextExtImpl : public Skeleton {
        std::map<std::string, std::shared_ptr<Object>> name2Object;

    public:
        NamingContextExtImpl(const std::shared_ptr<CORBA::ORB> orb) : Skeleton(orb) {}
        const char *_idlClassName() const override;

        void bind(const std::string &name, std::shared_ptr<Object> servant) {
            if (name2Object.contains(name)) {
                throw runtime_error(format("name \"{}\" is already bound to object", name));
            }
            name2Object[name] = servant;
        }

        Object *resolve(const std::string &name) {
            // console.log(`NamingContextImpl.resolve("${name}")`)
            auto servant = name2Object.find(name);
            if (servant == name2Object.end()) {
                cerr << format("name \"{}\" is not bound to an object", name) << endl;
                throw runtime_error(format("name \"{}\" is not bound to an object", name));
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
            // cerr << "NamingContextExtImpl::_call(" << operation << ", ...)" << endl;
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

Object::~Object() {}
const char *Object::_idlClassName() const { return nullptr; }

ORB::ORB() {}

string ORB::registerServant(Skeleton *servant) {
    CDREncoder encoder;
    encoder.ulonglong(++servantIdCounter);
    string oid(encoder.data(), encoder.length());
    servants[oid] = servant;
    return oid;
}

void ORB::bind(const std::string &id, std::shared_ptr<CORBA::Skeleton> const obj) {
    if (namingService == nullptr) {
        namingService = std::make_shared<NamingContextExtImpl>(shared_from_this());
        servants["NameService"] = namingService.get();
    }
    namingService->bind(id, obj);
}

void ORB::_socketRcvd(const uint8_t *buffer, size_t size) {
    cout << "RECEIVED" << endl;
    hexdump((unsigned char*)buffer, size);
    CORBA::CDRDecoder data((const char *)buffer, size);
    CORBA::GIOPDecoder decoder(data);
    auto type = decoder.scanGIOPHeader();
    switch (type) {
        case CORBA::GIOP_REQUEST: {
            auto request = decoder.scanRequestHeader();
            string objectKey(request->objectKey.data(), request->objectKey.length);  // FIXME: do not copy
            cout << "REQUEST(requestId=" << request->requestId << ", objectKey=" << hex << objectKey << ", " << request->method << ")" << endl;
            auto servant = servants.find(objectKey);  // FIXME: avoid string copy
            if (servant == servants.end()) {
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
                    auto length = encoder.buffer.offset; // FIXME: buffer.offset and buffer.lenght() differ
                    encoder.setGIOPHeader(GIOP_REPLY);
                    encoder.setReplyHeader(request->requestId, GIOP_NO_EXCEPTION);
                    cout << "TODO: SEND " << hex << length << " OCTETS" << endl;
                    sendChordata((void*)encoder.buffer.data(), length);
                    // send reply
                }
            } 
            catch (std::out_of_range e) {
                if (request->responseExpected) {
                    // send reply
                }
                cerr << "OUT OF RANGE: " << e.what() << endl;
            }
            catch (std::exception e) {
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
