#pragma once

#include <stdint.h>

#include <functional>
#include <string>

#include "cdr.hh"

namespace CORBA {

class ORB;
class Object;
class IOR;

enum class MessageType {
    REQUEST = 0,
    REPLY = 1,
    CANCEL_REQUEST = 2,
    LOCATE_REQUEST = 3,
    LOCATE_REPLY = 4,
    CLOSE_CONNECTION = 5,
    MESSAGE_ERROR = 6,
    FRAGMENT = 7
};

enum class ReplyStatus {
    NO_EXCEPTION = 0,
    USER_EXCEPTION = 1,
    SYSTEM_EXCEPTION = 2,
    LOCATION_FORWARD = 3,
    // since GIOP 1.2
    LOCATION_FORWARD_PERM = 4,
    NEEDS_ADDRESSING_MODE = 5
};

enum class ServiceId {
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
enum class ProfileId { TAG_INTERNET_IOP = 0, TAG_MULTIPLE_COMPONENTS = 1, TAG_SCCP_IOP = 2, TAG_UIPMC = 3, TAG_MOBILE_TERMINAL_IOP = 4 };

// CORBA 3.3 Part 2: 7.6.6 Standard IOP Components
enum class ComponentId {
    ORB_TYPE = 0,
    CODE_SETS = 1,
    POLICIES = 2,
    ALTERNATE_IIOP_ADDRESS = 3,
    COMPLETE_OBJECT_KEY = 5,
    ENDPOINT_ID_POSITION = 6,
    LOCATION_POLICY = 12,
    ASSOCIATION_OPTIONS = 13,
    SEC_NAME = 14,
    SPKM_1_SEC_MECH = 15,
    SPKM_2_SEC_MECH = 16,
    KerberosV5_SEC_MECH = 17,
    CSI_ECMA_Secret_SEC_MECH = 18,
    CSI_ECMA_Hybrid_SEC_MECH = 19,
    SSL_SEC_TRANS = 20,
    CSI_ECMA_Public_SEC_MECH = 21,
    GENERIC_SEC_MECH = 22,
    FIREWALL_TRANS = 23,
    SCCP_CONTACT_INFO = 24,
    JAVA_CODEBASE = 25,
    TRANSACTION_POLICY = 26,
    MESSAGE_ROUTERS = 30,
    OTS_POLICY = 31,
    INV_POLICY = 32,
    CSI_SEC_MECH_LIST = 33,
    NULL_TAG = 34,
    SECIOP_SEC_TRANS = 35,
    TLS_SEC_TRANS = 36,
    ACTIVITY_POLICY = 37,
    RMI_CUSTOM_MAX_STREAM_FORMAT = 38,
    GROUP = 39,
    GROUP_IIOP = 40,
    PASSTHRU_TRANS = 41,
    FIREWALL_PATH = 42,
    IIOP_SEC_TRANS = 43,
    DCE_STRING_BINDING = 100,
    DCE_BINDING_NAME = 101,
    DCE_NO_PIPES = 102,
    DCE_SEC_MECH = 103,
    INET_SEC_TRANS = 123
};

enum class AddressingDisposition { KEY_ADDR = 0, PROFILE_ADDR = 1, REFERENCE_ADDR = 2 };

struct GIOPHeader {
        char id[4];
        uint8_t majorVersion;
        uint8_t minorVersion;
        uint8_t endian;
        uint8_t type;
        uint32_t length;
};

struct RequestHeader {
        uint32_t requestId;
        bool responseExpected;
        blob_view objectKey;
        std::string_view operation;
};

struct ReplyHeader {
        uint32_t requestId;
        ReplyStatus replyStatus;
};

struct LocateRequest {
        uint32_t requestId;
        blob_view objectKey;
        LocateRequest(uint32_t requestId, const blob_view &objectKey) : requestId(requestId), objectKey(objectKey) {}
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
        inline void octet(u_int8_t value) { buffer.octet(value); }
        inline void ushort(uint16_t value) { buffer.ushort(value); }
        inline void ulong(uint32_t value) { buffer.ulong(value); }
        inline void ulonglong(uint64_t value) { buffer.ulonglong(value); }

        inline void string(const char *value) { buffer.string(value); }
        inline void string(const char *value, size_t size) { buffer.string(value, size); }
        inline void string(const std::string &value) { buffer.string(value); }
        inline void string(const std::string_view &value) { buffer.string(value); }

        inline void blob(const char *value, size_t size) { buffer.blob(value, size); }
        inline void blob(const std::string &value) { buffer.blob(value.data(), value.size()); }
        inline void blob(const std::string_view &value) { buffer.blob(value.data(), value.size()); }
        inline void blob(const CORBA::blob &value) { buffer.blob((const char*)value.data(), value.size()); }
        inline void blob(const CORBA::blob_view &value) { buffer.blob((const char*)value.data(), value.size()); }

        inline void endian() { buffer.endian(); }

        inline void reserveSize() { buffer.reserveSize(); }
        inline void fillInSize() { buffer.fillInSize(); }

        void object(const Object *object);
        void reference(const Object *object);
        void encapsulation(ComponentId type, std::function<void()> closure);
        void encapsulation(ProfileId type, std::function<void()> closure);
        void encapsulation(ServiceId type, std::function<void()> closure);
        void skipGIOPHeader();
        void skipReplyHeader();
        void setGIOPHeader(MessageType type);
        void setReplyHeader(uint32_t requestId, CORBA::ReplyStatus replyStatus);
        void encodeRequest(const CORBA::blob &objectKey, const std::string &operation, uint32_t requestId, bool responseExpected);
        void serviceContext();
};

class GIOPDecoder : public GIOPBase {
    public:
        CDRDecoder &buffer;
        MessageType m_type;
        size_t m_length;

        uint32_t requestId;
        ReplyStatus replyStatus;

        GIOPDecoder(CDRDecoder &buffer) : buffer(buffer) {}
        MessageType scanGIOPHeader();
        const RequestHeader *scanRequestHeader();
        const LocateRequest *scanLocateRequest();
        std::unique_ptr<ReplyHeader> scanReplyHeader();

        void serviceContext();

        // CORBA 3.4 Part 2, 9.3.3 Encapsulation
        // Used for ServiceContext, Profile and Component
        void encapsulation(std::function<void(ComponentId type)> closure);
        void encapsulation(std::function<void(ProfileId type)> closure);
        void encapsulation(std::function<void(ServiceId type)> closure);

        inline void endian() { buffer.endian(); }
        inline bool boolean() { return buffer.boolean(); }
        inline uint8_t octet()  { return buffer.octet(); }
        inline char8_t character() { return buffer.character(); }
        inline uint16_t ushort() { return buffer.ushort(); }
        inline uint32_t ulong() { return buffer.ulong(); }
        inline uint64_t ulonglong() { return buffer.ulonglong(); }
        // short
        // long
        // longlong
        // float
        // double

        // WHEN DECODING AS OUT -> string|blob
        // WHEN DECODING AS IN  -> string_view|blob_view
        inline CORBA::blob blob() { return buffer.blob(); }
        inline std::string string() { return buffer.string(); }
        inline std::string string(size_t length) { return buffer.string(length); }

        inline CORBA::blob_view blob_view() { return buffer.blob_view(); }
        inline std::string_view string_view() { return buffer.string_view(); }
        inline std::string_view string_view(size_t length) { return buffer.string_view(length); }
        // sequence
        // value
        // object
        // reference

        std::shared_ptr<Object> object(std::shared_ptr<CORBA::ORB> orb = std::shared_ptr<CORBA::ORB>());
        std::shared_ptr<IOR> reference(size_t length);
        std::shared_ptr<IOR> reference() { return reference(buffer.ulong()); }
};

}  // namespace CORBA
