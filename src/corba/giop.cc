#include "giop.hh"
#include "corba.hh"

#include <stdexcept>

using namespace std;

namespace CORBA {

void GIOPEncoder::object(const Object &object) {
    cerr << "GIOPEncoder::object(...)" << endl;
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
        std::cout << "SERVICE CONTEXT" << std::endl;
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
                throw std::runtime_error("Unsupported AddressingDisposition.");
            default:
                throw std::runtime_error("Unknown AddressingDisposition.");
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
    DataView objectKey;
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
                throw std::runtime_error("Unsupported AddressingDisposition.");
            default:
                throw std::runtime_error("Unknown AddressingDisposition.");
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
                    std::cout << "ServiceContext BI_DIR_IIOP" << std::endl;
                    break;
                case SecurityAttributeService:
                    std::cout << "ServiceContext SecurityAttributeService" << std::endl;
                    break;
                default:
                    std::cout << "ServiceContext " << serviceId << std::endl;
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