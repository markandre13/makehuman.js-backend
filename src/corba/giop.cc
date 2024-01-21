#include "giop.hh"
#include "corba.hh"

#include <stdexcept>
#include <iostream>
#include <format>

using namespace std;

namespace CORBA {

void GIOPEncoder::object(const Object *object) {
    cerr << "GIOPEncoder::object(...)" << endl;
    if (object == nullptr) {
        buffer.ulong(0);
        return;
    }
    auto stub = dynamic_cast<const Stub*>(object);
    if (stub != nullptr) {
        throw runtime_error("Can not serialize stub yet.");
    }
    auto skeleton = dynamic_cast<const Skeleton*>(object);
    if (skeleton != nullptr) {
        reference(object);
        return;
    }

    throw runtime_error("Can not serialize value type yet.");
}

// Interoperable Object Reference (IOR)
void GIOPEncoder::reference(const Object *object) {
    cerr << "GIOPEncoder::reference(...)" << endl;
    // const className = (object.constructor as any)._idlClassName()
    auto className = object->_idlClassName();

    auto host = "localhost";
    auto port = 9001;
    auto oid = format("IDL:{}:1.0", className);

    // type id
    buffer.string(oid);

    // tagged profile sequence
    buffer.ulong(1); // profileCount

    // profile id
    // 9.7.2 IIOP IOR Profiles
    buffer.ulong(0); // IOR.TAG.IOR.INTERNET_IOP
    buffer.reserveSize();
    buffer.endian();
    buffer.octet(majorVersion);
    buffer.octet(minorVersion);

    // FIXME: the object should know where it is located, at least, if it's a stub, skeleton is local
    buffer.string(host);
    buffer.ushort(port);
    // buffer.blob(object->id);

    // IIOP >= 1.1: components
    if (majorVersion != 1 || minorVersion != 0) {
        buffer.ulong(1); // component count = 1
        // buffer.beginEncapsulation(0); // TAG_ORB_TYPE (3.4 P 2, 7.6.6.1)
        buffer.ulong(0x4d313300); // "M13\0" as ORB Type ID for corba.js
        // buffer.endEncapsulation();
    }
    buffer.fillInSize();
}

MessageType GIOPDecoder::scanGIOPHeader() {
    auto header = reinterpret_cast<const GIOPHeader*>(buffer.data());
    if (memcmp(header->id, "GIOP", 4) != 0) {
        throw std::runtime_error("Missing GIOP Header");
    }
    buffer.setOffset(4);
    majorVersion = buffer.octet();
    minorVersion = buffer.octet();
    auto flags = buffer.octet();
    buffer.setLittleEndian(flags & 1);
    type = static_cast<MessageType>(buffer.octet());
    length = buffer.ulong();
    return type;
}

const RequestHeader* GIOPDecoder::scanRequestHeader() {
    // auto header = reinterpret_cast<const RequestHeader*>(buffer.data() + offset);
    // make_unique<RequestHeader>();
    if (majorVersion == 1 && minorVersion <= 1) {
        serviceContext();
        cout << "SERVICE CONTEXT" << endl;
    }
    auto header = new RequestHeader();
    header->requestId = buffer.ulong();
    auto responseFlags = buffer.octet();
    if (majorVersion == 1 && minorVersion <= 1) {
        header->responseExpected = responseFlags != 0;
    } else {
        switch (responseFlags) {
            case 0:  // SyncScope.NONE, WITH_TRANSPORT
                header->responseExpected = false;
                break;
            case 1:  // WITH_SERVER
                break;
            case 2:
                break;
            case 3:  // WITH_TARGET
                header->responseExpected = true;
                break;
        }
    }
    buffer.skip(3);  // RequestReserved

    if (majorVersion == 1 && minorVersion <= 1) {
        header->objectKey = buffer.blob();
    } else {
        auto addressingDisposition = buffer.ushort();
        switch (addressingDisposition) {
            case KeyAddr:
                header->objectKey = buffer.blob();
                break;
            case ProfileAddr:
            case ReferenceAddr:
                throw runtime_error("Unsupported AddressingDisposition.");
            default:
                throw runtime_error("Unknown AddressingDisposition.");
        }
    }

    // FIXME: rename 'method' into 'operation' as it's named in the CORBA standard
    header->method = buffer.string();

    if (majorVersion == 1 && minorVersion <= 1) {
        auto requestingPrincipalLength = buffer.ulong();
        // FIXME: this.offset += requestingPrincipalLength???
    } else {
        serviceContext();
        // header->serviceContext = serviceContext();
        // this.align(8)
    }
    return header;
}

const LocateRequest* GIOPDecoder::scanLocateRequest() {
    auto requestId = buffer.ulong();
    CDRDecoder objectKey;
    if (majorVersion == 1 && minorVersion <= 1) {
        objectKey = buffer.blob();
    } else {
        auto addressingDisposition = buffer.ushort();
        switch (addressingDisposition) {
            case KeyAddr:
                objectKey = buffer.blob();
                break;
            case ProfileAddr:
            case ReferenceAddr:
                throw runtime_error("Unsupported AddressingDisposition.");
            default:
                throw runtime_error("Unknown AddressingDisposition.");
        }
    }
    return new LocateRequest(requestId, objectKey);
}

void GIOPDecoder::serviceContext() {
    auto serviceContextListLength = buffer.ulong();
    for (auto i = 0; i < serviceContextListLength; ++i) {
        encapsulation([](uint32_t serviceId){
            switch(serviceId) {
                case CodeSets:
                    // std::cout << "ServiceContext CodeSets" << std::endl;
                    break;
                case BI_DIR_IIOP:
                    cout << "ServiceContext BI_DIR_IIOP" << endl;
                    break;
                case SecurityAttributeService:
                    cout << "ServiceContext SecurityAttributeService" << endl;
                    break;
                default:
                    cout << "ServiceContext " << serviceId << endl;
            }
        });
    }
}

void GIOPDecoder::encapsulation(std::function<void(uint32_t type)> closure) {
    auto type = buffer.ulong();
    auto size = buffer.ulong();
    auto nextOffset = buffer.getOffset() + size;
    auto lastEndian = buffer._endian;
    auto flags = buffer.octet();
    buffer.setLittleEndian(flags & 1);

    closure(type);

    buffer._endian = lastEndian;
    buffer.setOffset(nextOffset);
}

}  // namespace CORBA