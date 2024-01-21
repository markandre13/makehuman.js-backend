#include <stdint.h>

#include <functional>
#include <string>

#include "cdr.hh"

namespace CORBA {

enum GIOPMessageType {
    GIOP_REQUEST,
    GIOP_REPLY,
    GIOP_CANCEL_REQUEST,
    GIOP_LOCATE_REQUEST,
    GIOP_LOCATE_REPLY,
    GIOP_CLOSE_CONNECTION,
    GIOP_MESSAGE_ERROR,
    GIOP_FRAGMENT
};

enum GIOPReplyStatus {
    GIOP_NO_EXCEPTION = 0,
    GIOP_USER_EXCEPTION = 1,
    GIOP_SYSTEM_EXCEPTION = 2,
    GIOP_LOCATION_FORWARD = 3,
    // since GIOP 1.2
    GIOP_LOCATION_FORWARD_PERM = 4,
    GIOP_NEEDS_ADDRESSING_MODE = 5
};

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
        CDRDecoder objectKey;
        std::string_view method;
};

struct LocateRequest {
        uint32_t requestId;
        CDRDecoder objectKey;
        LocateRequest(uint32_t requestId, CDRDecoder objectKey) : requestId(requestId), objectKey(objectKey) {}
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
        CDREncoder buffer;
        void object(const Object *object);
        void reference(const Object *object);
        void encapsulation(uint32_t type, std::function<void()> closure);
        void skipGIOPHeader();
        void skipReplyHeader();
        void setGIOPHeader(GIOPMessageType type);
        void setReplyHeader(uint32_t requestId, uint32_t replyStatus);
};

class GIOPDecoder : public GIOPBase {
    public:
        CDRDecoder &buffer;
        GIOPMessageType type;
        size_t length;

        GIOPDecoder(CDRDecoder &buffer) : buffer(buffer) {}
        GIOPMessageType scanGIOPHeader();
        const RequestHeader *scanRequestHeader();
        const LocateRequest *scanLocateRequest();

        void serviceContext();

        // CORBA 3.4 Part 2, 9.3.3 Encapsulation
        // Used for ServiceContext, Profile and Component
        void encapsulation(std::function<void(uint32_t type)> closure);
};

}  // namespace CORBA
