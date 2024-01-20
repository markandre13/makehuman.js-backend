#include <stdint.h>

#include <functional>
#include <string>

#include "dataview.hh"

namespace CORBA {

enum MessageType { REQUEST, REPLY, CANCEL_REQUEST, LOCATE_REQUEST, LOCATE_REPLY, CLOSE_CONNECTION, MESSAGE_ERROR, FRAGMENT };

enum ServiceId {
    TransactionService = 0,
    CodeSets = 1,
    ChainBypassCheck = 2,
    ChainBypassInfo = 3,
    LogicalThreadId = 4,
    BI_DIR_IIOP = 5,
    SendingContextRunTime = 6,
    INVOCATION_POLICIES = 7,
    FORWARDED_IDENTITY = 8,
    UnknownExceptionInfo = 9,
    RTCorbaPriority = 10,
    RTCorbaPriorityRange = 11,
    FT_GROUP_VERSION = 12,
    FT_REQUEST = 13,
    ExceptionDetailMessage = 14,
    SecurityAttributeService = 15,  // CSIv2
    ActivityService = 16,
    RMICustomMaxStreamFormat = 17,
    ACCESS_SESSION_ID = 18,
    SERVICE_SESSION_ID = 19,
    FIREWALL_PATH = 20,
    FIREWALL_PATH_RESP = 21,

    // JacORB uses this as the last context to fill to an 8 byte boundary
    SERVICE_PADDING_CONTEXT = 0x4a414301  // "JAC\01"
};

struct GIOPHeader {
        char id[4];
        uint8_t majorVersion;
        uint8_t minorVersion;
        uint8_t endian;
        uint8_t type;
        uint32_t length;
};

struct Blob {
        uint32_t length;
        char data[];
};

enum AddressingDisposition { KeyAddr = 0, ProfileAddr = 1, ReferenceAddr = 2 };

struct RequestHeader {
        uint32_t requestId;
        bool responseExpected;
        DataView objectKey;
        std::string_view method;
};

struct LocateRequest {
        uint32_t requestId;
        DataView objectKey;
        LocateRequest(uint32_t requestId, DataView objectKey) : requestId(requestId), objectKey(objectKey) {}
};

class GIOPBase {
    public:
        unsigned majorVersion = 1;
        unsigned minorVersion = 2;

        const unsigned ENDIAN_LITTLE = 0;
        const unsigned ENDIAN_BIG = 1;

        // const FLOAT64_MAX = 1.7976931348623157e+308;
        // const FLOAT64_MIN = 2.2250738585072014e-308;
        // const TWO_TO_20 = 1048576;
        // const TWO_TO_32 = 4294967296;
        // const TWO_TO_52 = 4503599627370496;
};

class Object;

class GIOPEncoder : public GIOPBase {
    public:
        void object(const Object &object);
};

class GIOPDecoder : public GIOPBase {
    public:
        DataView &buffer;
        MessageType type;
        size_t length;

        GIOPDecoder(DataView &buffer) : buffer(buffer) {}
        MessageType scanGIOPHeader();
        const RequestHeader *scanRequestHeader();
        const LocateRequest *scanLocateRequest();

        void serviceContext();

        // CORBA 3.4 Part 2, 9.3.3 Encapsulation
        // Used for ServiceContext, Profile and Component
        void encapsulation(std::function<void(uint32_t type)> closure);
};

}  // namespace CORBA
