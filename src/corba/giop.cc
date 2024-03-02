#include "giop.hh"

#include <array>
#include <format>
#include <iostream>
#include <stdexcept>

#include "corba.hh"
#include "hexdump.hh"
#include "orb.hh"
#include "protocol.hh"
#include "blob.hh"

using namespace std;

namespace CORBA {

void GIOPEncoder::object(const CORBA::Object * object) {
    cerr << "GIOPEncoder::object(...)" << endl;
    if (dynamic_cast<const CORBA::Stub*>(object)) {
        std::println("GIOPEncoder::object(): STUB");
    }
    if (dynamic_cast<const CORBA::Skeleton*>(object)) {
        std::println("GIOPEncoder::object(): SKELETON");
    }
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
    auto reference = dynamic_cast<const ObjectReference*>(object);
    if (reference != nullptr) {
        throw runtime_error("Can not serialize object reference yet.");
        // reference(object);
        return;
    }
    throw runtime_error("Can not serialize value type yet.");
}

// Interoperable Object Reference (IOR)
void GIOPEncoder::reference(const Object* object) {
    cerr << "GIOPEncoder::reference(...) ENTER" << endl;
    // const className = (object.constructor as any)._idlClassName()
    auto className = object->repository_id();
    // if (className == nullptr) {
    //     cerr << "GIOPEncoder::reference(...) [1]" << endl;
    //     throw runtime_error("GIOPEncoder::reference(): _idlClassName() must not return nullptr");
    // }

    // type id
    string(className);

    // tagged profile sequence
    ulong(1);  // profileCount

    // profile id
    // 9.7.2 IIOP IOR Profiles
    ulong(0);  // IOR.TAG.IOR.INTERNET_IOP
    reserveSize();
    endian();
    octet(majorVersion);
    octet(minorVersion);

    if (connection == nullptr) {
        cerr << "GIOPEncoder::reference(...) [2]" << endl;
        throw runtime_error("GIOPEncoder::reference(...): the encoder has no connection and can not be reached over the network");
    }
    string(connection->localAddress());
    ushort(connection->localPort());
    blob(object->get_object_key());

    // IIOP >= 1.1: components
    if (majorVersion != 1 || minorVersion != 0) {
        ulong(1);               // component count = 1
        encapsulation(ComponentId::ORB_TYPE, [this]() {    // 0:  TAG_ORB_TYPE (3.4 P 2, 7.6.6.1)
            ulong(0x4d313300);  // "M13\0" as ORB Type ID for corba.js
        });
    }
    fillInSize();
    cerr << "GIOPEncoder::reference(...) LEAVE" << endl;
}

void GIOPEncoder::encapsulation(ComponentId type, std::function<void()> closure) {
    buffer.ulong(static_cast<uint32_t>(type));
    buffer.reserveSize();
    buffer.endian();
    closure();
    buffer.fillInSize();
}
void GIOPEncoder::encapsulation(ProfileId type, std::function<void()> closure) {
    buffer.ulong(static_cast<uint32_t>(type));
    buffer.reserveSize();
    buffer.endian();
    closure();
    buffer.fillInSize();
}
void GIOPEncoder::encapsulation(ServiceId type, std::function<void()> closure) {
    buffer.ulong(static_cast<uint32_t>(type));
    buffer.reserveSize();
    buffer.endian();
    closure();
    buffer.fillInSize();
}

void GIOPEncoder::skipGIOPHeader() { buffer.offset = 10; }
void GIOPEncoder::skipReplyHeader() {
    buffer.offset = 24;  // this does not work!!! anymore with having a variable length service context!!!
}
void GIOPEncoder::setGIOPHeader(MessageType type) {
    auto offset = buffer.offset;
    buffer.offset = 0;
    buffer.octet('G');
    buffer.octet('I');
    buffer.octet('O');
    buffer.octet('P');
    buffer.octet(majorVersion);
    buffer.octet(minorVersion);
    buffer.endian();
    buffer.octet(static_cast<uint8_t>(type));
    cout << "GIOPHeader length = " << hex << offset - 12 << endl;
    buffer.ulong(offset - 12);
    buffer.offset = offset;
}
void GIOPEncoder::setReplyHeader(uint32_t requestId, ReplyStatus replyStatus) {
    skipGIOPHeader();
    // fixme: create and use version methods like isVersionLessThan(1,2) or isVersionVersionGreaterEqual(1,2)
    if (majorVersion == 1 && minorVersion < 2) {
        // this.serviceContext()
        buffer.ulong(0);  // skipReplyHeader needs a fixed size service context
    }
    buffer.ulong(requestId);
    buffer.ulong(static_cast<uint32_t>(replyStatus));
    if (majorVersion == 1 && minorVersion >= 2) {
        // this.serviceContext();
        buffer.ulong(0);  // skipReplyHeader needs a fixed size service context
    }
}

void GIOPEncoder::encodeRequest(const CORBA::blob &objectKey, const std::string &operation, uint32_t requestId, bool responseExpected) {
    skipGIOPHeader();

    if (majorVersion == 1 && minorVersion <= 1) {
        serviceContext();
    }
    ulong(requestId);
    if (majorVersion == 1 && minorVersion <= 1) {
        octet(responseExpected ? 1 : 0);
    } else {
        octet(responseExpected ? 3 : 0);
    }
    buffer.offset += 3;

    if (majorVersion == 1 && minorVersion <= 1) {
        this->blob(objectKey);
    } else {
        ushort(static_cast<uint16_t>(AddressingDisposition::KEY_ADDR));
        this->blob(objectKey);
    }

    string(operation);
    if (majorVersion == 1 && minorVersion <= 1) {
        ulong(0);  // Requesting Principal length
    } else {
        serviceContext();
        buffer.align8();  // alignAndReserve(8);
    }
}

void GIOPEncoder::serviceContext() {
    // TODO: remove this, this happens only in tests
    // if (connection == nullptr) {
    ulong(0);
    return;
    // }

    // auto count = 1;
    // let initialToken
    // if (this.connection.orb.outgoingAuthenticator) {
    //     initialToken = this.connection.orb.outgoingAuthenticator(this.connection)
    //     if (initialToken) {
    //         ++count
    //     }
    // }

    // ulong(count); // count

    // CORBA 3.4 Part 2, 9.8.1 Bi-directional IIOP Service Context
    // TODO: send listen point only once per connection
    // beginEncapsulation(ServiceId.BI_DIR_IIOP);
    // ulong(1); // number of listen points
    // string(connection->getLocalAddress());
    // ushort(connection->getLocalPort());
    // endEncapsulation();

    // if (initialToken) {
    //     this.establishSecurityContext(initialToken);
    // }

    /*
    this.beginEncapsulation(ServiceId.CodeSets)
    // this.ulong(0x00010001) // ISO-8859-1
    this.ulong(0x05010001) // charset_id : UTF-8
    this.ulong(0x00010109) // wcharset_id: UTF-16
    this.endEncapsulation()
    */
}

////////////////////////////////////////////

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
    m_type = static_cast<MessageType>(buffer.octet());
    m_length = buffer.ulong();
    return m_type;
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
        header->objectKey = buffer.blob_view();
    } else {
        auto addressingDisposition = static_cast<AddressingDisposition>(buffer.ushort());
        switch (addressingDisposition) {
            case AddressingDisposition::KEY_ADDR:
                header->objectKey = buffer.blob_view();
                break;
            case AddressingDisposition::PROFILE_ADDR:
            case AddressingDisposition::REFERENCE_ADDR:
                throw runtime_error("Unsupported AddressingDisposition.");
            default:
                throw runtime_error("Unknown AddressingDisposition.");
        }
    }
    // cout << "REQUEST objectKey size = " << header->objectKey.length << endl;
    header->operation = buffer.string_view();

    if (majorVersion == 1 && minorVersion <= 1) {
        auto requestingPrincipalLength = buffer.ulong();
        // FIXME: this.offset += requestingPrincipalLength???
    } else {
        serviceContext();
        // header->serviceContext = serviceContext();
        buffer.align8();
    }
    return header;
}

const LocateRequest* GIOPDecoder::scanLocateRequest() {
    this->requestId = buffer.ulong();
    CORBA::blob_view objectKey;
    if (majorVersion == 1 && minorVersion <= 1) {
        objectKey = buffer.blob_view();
    } else {
        auto addressingDisposition = static_cast<AddressingDisposition>(buffer.ushort());
        switch (addressingDisposition) {
            case AddressingDisposition::KEY_ADDR:
                objectKey = buffer.blob_view();
                break;
            case AddressingDisposition::PROFILE_ADDR:
            case AddressingDisposition::REFERENCE_ADDR:
                throw runtime_error("Unsupported AddressingDisposition.");
            default:
                throw runtime_error("Unknown AddressingDisposition.");
        }
    }
    return new LocateRequest(this->requestId, objectKey);  // unique_ptr
}

unique_ptr<ReplyHeader> GIOPDecoder::scanReplyHeader() {
    if (majorVersion == 1 && minorVersion <= 1) {
        serviceContext();
    }
    this->requestId = buffer.ulong();
    this->replyStatus = static_cast<ReplyStatus>(buffer.ulong());
    if (majorVersion == 1 && minorVersion >= 2) {
        serviceContext();
    }
    return make_unique<ReplyHeader>(this->requestId, this->replyStatus);
}

void GIOPDecoder::serviceContext() {
    auto serviceContextListLength = buffer.ulong();
    for (size_t i = 0; i < serviceContextListLength; ++i) {
        encapsulation([](ServiceId serviceId) {
            switch (serviceId) {
                case ServiceId::CodeSets:
                    // std::cout << "ServiceContext CodeSets" << std::endl;
                    break;
                case ServiceId::BI_DIR_IIOP:
                    cout << "ServiceContext BI_DIR_IIOP" << endl;
                    break;
                case ServiceId::SecurityAttributeService:
                    cout << "ServiceContext SecurityAttributeService" << endl;
                    break;
                default:
                    cout << "ServiceContext " << static_cast<unsigned>(serviceId) << endl;
            }
        });
    }
}

void GIOPDecoder::encapsulation(std::function<void(ComponentId type)> closure) {
    auto type = static_cast<ComponentId>(buffer.ulong());
    auto size = buffer.ulong();
    auto nextOffset = buffer.getOffset() + size;
    auto lastEndian = buffer._endian;
    auto flags = buffer.octet();
    buffer.setLittleEndian(flags & 1);

    closure(type);

    buffer._endian = lastEndian;
    buffer.setOffset(nextOffset);
}
void GIOPDecoder::encapsulation(std::function<void(ProfileId type)> closure) {
    auto type = static_cast<ProfileId>(buffer.ulong());
    auto size = buffer.ulong();
    auto nextOffset = buffer.getOffset() + size;
    auto lastEndian = buffer._endian;
    auto flags = buffer.octet();
    buffer.setLittleEndian(flags & 1);

    closure(type);

    buffer._endian = lastEndian;
    buffer.setOffset(nextOffset);
}
void GIOPDecoder::encapsulation(std::function<void(ServiceId type)> closure) {
    auto type = static_cast<ServiceId>(buffer.ulong());
    auto size = buffer.ulong();
    auto nextOffset = buffer.getOffset() + size;
    auto lastEndian = buffer._endian;
    auto flags = buffer.octet();
    buffer.setLittleEndian(flags & 1);

    closure(type);

    buffer._endian = lastEndian;
    buffer.setOffset(nextOffset);
}

std::shared_ptr<Object> GIOPDecoder::object(ORB *orb) {  // const string typeInfo, bool isValue = false) {
    auto code = buffer.ulong();
    auto objectOffset = buffer.m_offset - 4;

    if (code == 0) {
        cerr << "[2]" << endl;
        return std::shared_ptr<Object>();
    }

    if ((code & 0xffffff00) == 0x7fffff00) {
        throw runtime_error("GIOPDecoder::object(): value types are not implemented yet");
    }

    if (code == 0xffffffff) {
        throw runtime_error("GIOPDecoder::object(): pointers are not implemented yet");
    }

    if (code < 0x7fffff00) {
        if (orb == nullptr) {
            throw runtime_error("GIOPDecoder::object(orb): orb must not be null");
        }
        auto ref = reference(code);
        cerr << "GOT IOR " << ref->oid << " " << ref->objectKey << endl;
        return make_shared<ObjectReference>(orb, ref->oid, ref->host, ref->port, ref->get_object_key());
    }
    throw runtime_error(format("GIOPDecoder: Unsupported value with CORBA tag {:#x}", code));
}

struct ORBTypeName {
        uint32_t from;
        uint32_t to;
        const char* name;
};

static array<ORBTypeName, 35> orbTypeNames{{{0x48500000, 0x4850000f, "Hewlett Packard"},
                                            {0x49424d00, 0x49424d0f, "IBM"},
                                            {0x494c5500, 0x494c55ff, "Xerox"},
                                            {0x49534900, 0x4953490f, "AdNovum Informatik AG"},
                                            {0x56495300, 0x5649530f, "Borland (VisiBroker)"},
                                            {0x4f495300, 0x4f4953ff, "Objective Interface Systems"},
                                            {0x46420000, 0x4642000f, "FloorBoard Software"},
                                            {0x4E4E4E56, 0x4E4E4E56, "Rogue Wave"},
                                            {0x4E550000, 0x4E55000f, "Nihon Unisys, Ltd"},
                                            {0x4A424B52, 0x4A424B52, "SilverStream Software"},
                                            {0x54414f00, 0x54414f00, "Center for Distributed Object Computing, Washington University"},
                                            {0x4C434200, 0x4C43420F, "2AB"},
                                            {0x41505831, 0x41505831, "Informatik 4, Univ. of Erlangen-Nuernberg"},
                                            {0x4f425400, 0x4f425400, "ORBit"},
                                            {0x47534900, 0x4753490f, "GemStone Systems, Inc."},
                                            {0x464a0000, 0x464a000f, "Fujitsu Limited"},
                                            {0x4E534440, 0x4E53444F, "Compaq Computer"},
                                            {0x4f425f00, 0x4f425f0f, "TIBCO"},
                                            {0x4f414b00, 0x4f414b0f, "Camros Corporation"},
                                            {0x41545400, 0x4154540f, "AT&T Laboratories, Cambridge (OmniORB)"},
                                            {0x4f4f4300, 0x4f4f430f, "IONA Technologies"},
                                            {0x4e454300, 0x4e454303, "NEC Corporation"},
                                            {0x424c5500, 0x424c550f, "Berry Software"},
                                            {0x56495400, 0x564954ff, "Vitra"},
                                            {0x444f4700, 0x444f47ff, "Exoffice Technologies"},
                                            {0xcb0e0000, 0xcb0e00ff, "Chicago Board of Exchange (CBOE)"},
                                            {0x4A414300, 0x4A41430f, "FU Berlin Institut für Informatik (JAC)"},
                                            {0x58545240, 0x5854524F, "Xtradyne Technologies AG"},
                                            {0x54475800, 0x54475803, "Top Graph'X"},
                                            {0x41646100, 0x41646103, "AdaOS project"},
                                            {0x4e4f4b00, 0x4e4f4bff, "Nokia"},
                                            {0x53414E00, 0x53414E0f, "Sankhya Technologies Private Limited, India"},
                                            {0x414E4400, 0x414E440f, "Androsoft GmbH"},
                                            {0x42424300, 0x4242430f, "Bionic Buffalo Corporation"},
                                            {0x4d313300, 0x4d313300, "corba.js"}}};

struct Reference {
        std::string oid;
        std::string host;
        uint16_t port;
        std::string objectKey;
};

shared_ptr<ObjectReference> GIOPDecoder::reference(size_t length) {
    Reference data;
    // auto data = make_sunique<ObjectReference>();

    // struct IOR, field: string type_id ???
    data.oid = buffer.string(length);
    // console.log(`IOR: oid: '${data.oid}'`)

    // struct IOR, field: TaggedProfileSeq profiles ???
    auto profileCount = buffer.ulong();
    // console.log(`oid: '${oid}', tag count=${tagCount}`)
    for (uint32_t i = 0; i < profileCount; ++i) {
        encapsulation([this, &data](ProfileId profileId) {
            switch (profileId) {
                // CORBA 3.3 Part 2: 9.7.2 IIOP IOR Profiles
                case ProfileId::TAG_INTERNET_IOP: {
                    // console.log(`Internet IOP Component, length=${profileLength}`)
                    auto iiopMajorVersion = buffer.octet();
                    auto iiopMinorVersion = buffer.octet();
                    // if (iiopMajorVersion !== 1 || iiopMinorVersion > 1) {
                    //     throw Error(`Unsupported IIOP ${iiopMajorVersion}.${iiopMinorVersion}. Currently only IIOP
                    //     ${GIOPBase.MAJOR_VERSION}.${GIOPBase.MINOR_VERSION} is implemented.`)
                    // }
                    data.host = buffer.string();
                    data.port = buffer.ushort();
                    auto objectKey = buffer.blob();
                    data.objectKey = std::string((const char *)objectKey.data(), objectKey.size());
                    // console.log(`IOR: IIOP(version: ${iiopMajorVersion}.${iiopMinorVersion}, host: ${data.host}:${data.port}, objectKey: ${data.objectKey})`)
                    // FIXME: use utility function to compare version!!! better use hex: version >= 0x0101
                    if (iiopMajorVersion == 1 && iiopMinorVersion != 0) {
                        // TaggedComponentSeq
                        auto n = buffer.ulong();
                        // console.log(`IOR: ${n} components`)
                        for (uint32_t i = 0; i < n; ++i) {
                            auto id = static_cast<ComponentId>(buffer.ulong()); // TODO: make this a method called componentId
                            auto length = buffer.ulong();
                            auto nextOffset = buffer.m_offset + length;
                            switch (id) {
                                case ComponentId::ORB_TYPE: {
                                    auto typeCount = buffer.ulong();
                                    for (uint32_t j = 0; j < typeCount; ++j) {
                                        auto orbType = buffer.ulong();
                                        const char* name = nullptr;
                                        for (const auto& orbTypeName : orbTypeNames) {
                                            if (orbTypeName.from <= orbType && orbType <= orbTypeName.to) {
                                                name = orbTypeName.name;
                                                break;
                                            }
                                        }
                                        if (name == nullptr) {
                                            cerr << format("IOR: component[{}] = ORB_TYPE {:x}", i, orbType) << endl;
                                        } else {
                                            cerr << format("IOR: component[{}] = ORB_TYPE {}", i, name) << endl;
                                        }
                                    }
                                } break;
                                case ComponentId::CODE_SETS:
                                    // Corba 3.4, Part 2, 7.10.2.4 CodeSet Component of IOR Multi-Component Profile
                                    // console.log(`IOR: component[${i}] = CODE_SETS`)
                                    break;
                                case ComponentId::POLICIES:
                                    // console.log(`IOR: component[${i}] = POLICIES`)
                                    break;
                                default:
                                    // console.log(`IOR: component[${i}] = ${id} (0x${id.toString(16)})`)
                            }
                            buffer.m_offset = nextOffset;
                        }
                    }
                } break;
                default: {
                    auto id = static_cast<unsigned>(profileId);
                    cerr << format("IOR: Unhandled profile id={} {:x}", id, id) << endl;
                }
            }
        });
    }
    auto b = CORBA::blob_view(data.objectKey);
    return make_shared<ObjectReference>(nullptr, data.oid, data.host, data.port, b);
}

}  // namespace CORBA