#pragma once

#include <stdint.h>

#include <functional>
#include <string>

#include "cdr.hh"

namespace CORBA {

class ORB;
class Object;
class ObjectReference;

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

// CORBA 3.3 Part 2: 7.6.4 Standard IOR Profiles
enum ProfileId { TAG_INTERNET_IOP = 0, TAG_MULTIPLE_COMPONENTS = 1, TAG_SCCP_IOP = 2, TAG_UIPMC = 3, TAG_MOBILE_TERMINAL_IOP = 4 };

// CORBA 3.3 Part 2: 7.6.6 Standard IOP Components
enum ComponentId {
    TAG_ORB_TYPE = 0,
    TAG_CODE_SETS = 1,
    TAG_POLICIES = 2,
    TAG_ALTERNATE_IIOP_ADDRESS = 3,
    TAG_COMPLETE_OBJECT_KEY = 5,
    TAG_ENDPOINT_ID_POSITION = 6,
    TAG_LOCATION_POLICY = 12,
    TAG_ASSOCIATION_OPTIONS = 13,
    TAG_SEC_NAME = 14,
    TAG_SPKM_1_SEC_MECH = 15,
    TAG_SPKM_2_SEC_MECH = 16,
    TAG_KerberosV5_SEC_MECH = 17,
    TAG_CSI_ECMA_Secret_SEC_MECH = 18,
    TAG_CSI_ECMA_Hybrid_SEC_MECH = 19,
    TAG_SSL_SEC_TRANS = 20,
    TAG_CSI_ECMA_Public_SEC_MECH = 21,
    TAG_GENERIC_SEC_MECH = 22,
    TAG_FIREWALL_TRANS = 23,
    TAG_SCCP_CONTACT_INFO = 24,
    TAG_JAVA_CODEBASE = 25,
    TAG_TRANSACTION_POLICY = 26,
    TAG_MESSAGE_ROUTERS = 30,
    TAG_OTS_POLICY = 31,
    TAG_INV_POLICY = 32,
    TAG_CSI_SEC_MECH_LIST = 33,
    TAG_NULL_TAG = 34,
    TAG_SECIOP_SEC_TRANS = 35,
    TAG_TLS_SEC_TRANS = 36,
    TAG_ACTIVITY_POLICY = 37,
    TAG_RMI_CUSTOM_MAX_STREAM_FORMAT = 38,
    TAG_GROUP = 39,
    TAG_GROUP_IIOP = 40,
    TAG_PASSTHRU_TRANS = 41,
    TAG_FIREWALL_PATH = 42,
    TAG_IIOP_SEC_TRANS = 43,
    TAG_DCE_STRING_BINDING = 100,
    TAG_DCE_BINDING_NAME = 101,
    TAG_DCE_NO_PIPES = 102,
    TAG_DCE_SEC_MECH = 103,
    TAG_INET_SEC_TRANS = 123
};

enum AddressingDisposition { GIOP_KEY_ADDR = 0, GIOP_PROFILE_ADDR = 1, GIOP_REFERENCE_ADDR = 2 };

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

struct RequestHeader {
        uint32_t requestId;
        bool responseExpected;
        std::string objectKey;
        std::string method;
};

struct ReplyHeader {
        uint32_t requestId;
        GIOPReplyStatus replyStatus;
};

struct LocateRequest {
        uint32_t requestId;
        std::string objectKey;
        LocateRequest(uint32_t requestId, const std::string &objectKey) : requestId(requestId), objectKey(objectKey) {}
};

namespace detail {
class Connection;
}

class GIOPBase {
    public:
        GIOPBase(detail::Connection *connection = nullptr) : connection(connection) {}
        unsigned majorVersion = 1;
        unsigned minorVersion = 2;

        const unsigned ENDIAN_LITTLE = 0;
        const unsigned ENDIAN_BIG = 1;

        detail::Connection *connection;

        // const FLOAT64_MAX = 1.7976931348623157e+308;
        // const FLOAT64_MIN = 2.2250738585072014e-308;
        // const TWO_TO_20 = 1048576;
        // const TWO_TO_32 = 4294967296;
        // const TWO_TO_52 = 4503599627370496;
};

class GIOPEncoder : public GIOPBase {
    public:
        GIOPEncoder(detail::Connection *connection = nullptr) : GIOPBase(connection) { this->connection = connection; }

        CDREncoder buffer;
        inline void boolean(bool value) { buffer.boolean(value); }
        void octet(u_int8_t value) { buffer.octet(value); }
        void ushort(uint16_t value) { buffer.ushort(value); }
        void ulong(uint32_t value) { buffer.ulong(value); }
        void ulonglong(uint64_t value) { buffer.ulonglong(value); }

        void string(const char *value) { buffer.string(value); }
        void string(const char *value, size_t size) { buffer.string(value); }
        void string(const std::string &value) { buffer.string(value); }
        void blob(const char *value, size_t size) { buffer.blob(value, size); }
        void blob(const std::string &value) { buffer.blob(value.data(), value.size()); }
        void endian() { buffer.endian(); }

        void reserveSize() { buffer.reserveSize(); }
        void fillInSize() { buffer.fillInSize(); }

        void object(const Object *object);
        void reference(const Object *object);
        void encapsulation(uint32_t type, std::function<void()> closure);
        void skipGIOPHeader();
        void skipReplyHeader();
        void setGIOPHeader(GIOPMessageType type);
        void setReplyHeader(uint32_t requestId, uint32_t replyStatus);
        void encodeRequest(std::string objectKey, std::string operation, uint32_t requestId, bool responseExpected);
        void serviceContext();
};

class GIOPDecoder : public GIOPBase {
    public:
        CDRDecoder &buffer;
        GIOPMessageType m_type;
        size_t m_length;

        GIOPDecoder(CDRDecoder &buffer) : buffer(buffer) {}
        GIOPMessageType scanGIOPHeader();
        const RequestHeader *scanRequestHeader();
        const LocateRequest *scanLocateRequest();
        std::unique_ptr<ReplyHeader> scanReplyHeader();

        void serviceContext();

        // CORBA 3.4 Part 2, 9.3.3 Encapsulation
        // Used for ServiceContext, Profile and Component
        void encapsulation(std::function<void(uint32_t type)> closure);

        void *object(CORBA::ORB *);  // const string typeInfo, bool isValue = false) {
        std::shared_ptr<ObjectReference> reference(size_t length);
        std::shared_ptr<ObjectReference> reference() { return reference(buffer.ulong()); }
};

}  // namespace CORBA
