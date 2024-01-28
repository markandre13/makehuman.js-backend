#include "giop.hh"

#include <array>
#include <format>
#include <iostream>
#include <stdexcept>

#include "corba.hh"

using namespace std;

namespace CORBA {

void GIOPEncoder::object(const ObjectBase* object) {
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
void GIOPEncoder::reference(const ObjectBase* object) {
    cerr << "GIOPEncoder::reference(...)" << endl;
    // const className = (object.constructor as any)._idlClassName()
    auto className = object->_idlClassName();
    if (className == nullptr) {
        throw runtime_error("GIOPEncoder::reference(): _idlClassName() must not return nullptr");
    }

    auto host = "localhost";
    auto port = 9001;
    auto oid = format("IDL:{}:1.0", className);

    // type id
    buffer.string(oid);

    // tagged profile sequence
    buffer.ulong(1);  // profileCount

    // profile id
    // 9.7.2 IIOP IOR Profiles
    buffer.ulong(0);  // IOR.TAG.IOR.INTERNET_IOP
    buffer.reserveSize();
    buffer.endian();
    buffer.octet(majorVersion);
    buffer.octet(minorVersion);

    // FIXME: the object should know where it is located, at least, if it's a stub, skeleton is local
    buffer.string(host);
    buffer.ushort(port);
    buffer.blob((const char*)object->id.data(), object->id.size());

    // IIOP >= 1.1: components
    if (majorVersion != 1 || minorVersion != 0) {
        buffer.ulong(1);               // component count = 1
        encapsulation(0, [this]() {    // 0:  TAG_ORB_TYPE (3.4 P 2, 7.6.6.1)
            buffer.ulong(0x4d313300);  // "M13\0" as ORB Type ID for corba.js
        });
    }
    buffer.fillInSize();
}

void GIOPEncoder::encapsulation(uint32_t type, std::function<void()> closure) {
    buffer.ulong(type);
    buffer.reserveSize();
    buffer.endian();
    closure();
    buffer.fillInSize();
}

void GIOPEncoder::skipGIOPHeader() { buffer.offset = 10; }
void GIOPEncoder::skipReplyHeader() {
    buffer.offset = 24;  // this does not work!!! anymore with having a variable length service context!!!
}
void GIOPEncoder::setGIOPHeader(GIOPMessageType type) {
    auto offset = buffer.offset;
    buffer.offset = 0;
    buffer.octet('G');
    buffer.octet('I');
    buffer.octet('O');
    buffer.octet('P');
    buffer.octet(majorVersion);
    buffer.octet(minorVersion);
    buffer.endian();
    buffer.octet(type);
    cout << "GIOPHeader length = " << hex << offset - 12 << endl;
    buffer.ulong(offset - 12);
    buffer.offset = offset;
}
void GIOPEncoder::setReplyHeader(uint32_t requestId, uint32_t replyStatus) {
    skipGIOPHeader();
    // fixme: create and use version methods like isVersionLessThan(1,2) or isVersionVersionGreaterEqual(1,2)
    if (majorVersion == 1 && minorVersion < 2) {
        // this.serviceContext()
        buffer.ulong(0);  // skipReplyHeader needs a fixed size service context
    }
    buffer.ulong(requestId);
    buffer.ulong(replyStatus);
    if (majorVersion == 1 && minorVersion >= 2) {
        // this.serviceContext();
        buffer.ulong(0);  // skipReplyHeader needs a fixed size service context
    }
}

GIOPMessageType GIOPDecoder::scanGIOPHeader() {
    auto header = reinterpret_cast<const GIOPHeader*>(buffer.data());
    if (memcmp(header->id, "GIOP", 4) != 0) {
        throw std::runtime_error("Missing GIOP Header");
    }
    buffer.setOffset(4);
    majorVersion = buffer.octet();
    minorVersion = buffer.octet();
    auto flags = buffer.octet();
    buffer.setLittleEndian(flags & 1);
    type = static_cast<GIOPMessageType>(buffer.octet());
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
    cout << "REQUEST objectKey size = " << header->objectKey.length << endl;

    // FIXME: rename 'method' into 'operation' as it's named in the CORBA standard
    header->method = buffer.string();

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
        encapsulation([](uint32_t serviceId) {
            switch (serviceId) {
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

void* GIOPDecoder::object(ORB *orb) {  // const string typeInfo, bool isValue = false) {
    auto code = buffer.ulong();
    auto objectOffset = buffer.offset - 4;

    if (code == 0) {
        cerr << "[2]" << endl;
        return nullptr;
    }

    if ((code & 0xffffff00) == 0x7fffff00) {
        throw runtime_error("GIOPDecoder::object(): value types are not implemented yet");
    }

    if (code == 0xffffffff) {
        throw runtime_error("GIOPDecoder::object(): pointers are not implemented yet");
    }

    if (code < 0x7fffff00) {
        cerr << "got reference" << endl;
        auto ref = reference(code);

        // if (ref.host == this.connection.localAddress && ref.port == this.connection.localPort) {
        //     return this.connection.orb.servants.get(ref.objectKey)
        // }

        cerr << "GOT OBJECT " << ref->oid << " " << ref->objectKey.toString() << endl;

        // HAVE A LOOK AT WHAT ORGINAL CORBA RETURNS HERE, ME THINKS IT'S JUST THE OBJECT REFERENCE
        // AND THE REST IS DONE IN _narrow()

        // CORBA::Object_var ORB::string_to_object(const char+)
        // In Orbit, Objects have Connections

        // in MICO
        // class Object : public ServerlessObject {
        //    IOR *ior;
        //    IOR *fwd_ior;
        //    ORB_ptr orb;
        //    string indent;
        // }

        // class C_stub: C, ::CORBA::ExtInterfaceDef_stub

        // TODO: this belongs elsewhere
        // let object = this.connection.stubsById.get(ref.objectKey)
        // if (object !== undefined)
        //     return object
        // const shortName = reference.oid.substring(4, reference.oid.length - 4)
        // let aStubClass = this.connection.orb.stubsByName.get(shortName)
        // if (aStubClass === undefined) {
        //     // throw Error(`ORB: no stub registered for OID '${reference.oid}' (${shortName})`)
        //     throw new OBJECT_ADAPTER(0x4f4d0003, CompletionStatus.NO)
        // }
        // object = new aStubClass(this.connection.orb, reference.objectKey, this.connection)
        // this.connection.stubsById.set(reference.objectKey, object!)
        // return object
    }

    throw runtime_error(format("GIOPDecoder: Unsupported value with CORBA tag {:#x}", code));
}

struct ORBTypeName {
        uint32_t from;
        uint32_t to;
        const char* name;
};

static array<ORBTypeName, 35> orbTypeNames {{
    {0x48500000, 0x4850000f, "Hewlett Packard"},
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
    {0x4d313300, 0x4d313300, "corba.js"}
}};

// returns ObjectReference
unique_ptr<ObjectReference> GIOPDecoder::reference(size_t length) {
    auto data = make_unique<ObjectReference>();

    // struct IOR, field: string type_id ???
    data->oid = buffer.string(length);
    // console.log(`IOR: oid: '${data.oid}'`)

    // struct IOR, field: TaggedProfileSeq profiles ???
    auto profileCount = buffer.ulong();
    // console.log(`oid: '${oid}', tag count=${tagCount}`)
    for (uint32_t i = 0; i < profileCount; ++i) {
        auto _data = data.get();
        encapsulation([this, _data](uint32_t profileId) {
            switch (profileId) {
                // CORBA 3.3 Part 2: 9.7.2 IIOP IOR Profiles
                case TAG_INTERNET_IOP: {
                        // console.log(`Internet IOP Component, length=${profileLength}`)
                    auto iiopMajorVersion = buffer.octet();
                    auto iiopMinorVersion = buffer.octet();
                    // if (iiopMajorVersion !== 1 || iiopMinorVersion > 1) {
                    //     throw Error(`Unsupported IIOP ${iiopMajorVersion}.${iiopMinorVersion}. Currently only IIOP
                    //     ${GIOPBase.MAJOR_VERSION}.${GIOPBase.MINOR_VERSION} is implemented.`)
                    // }
                    _data->host = buffer.string();
                    _data->port = buffer.ushort();
                    _data->objectKey = buffer.blob();
                    // console.log(`IOR: IIOP(version: ${iiopMajorVersion}.${iiopMinorVersion}, host: ${data.host}:${data.port}, objectKey: ${data.objectKey})`)
                    // FIXME: use utility function to compare version!!! better use hex: version >= 0x0101
                    if (iiopMajorVersion == 1 && iiopMinorVersion != 0) {
                        // TaggedComponentSeq
                        auto n = buffer.ulong();
                        // console.log(`IOR: ${n} components`)
                        for (uint32_t i = 0; i < n; ++i) {
                            auto id = buffer.ulong();
                            auto length = buffer.ulong();
                            auto nextOffset = buffer.offset + length;
                            switch (id) {
                                case TAG_ORB_TYPE: {
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
                                case TAG_CODE_SETS:
                                    // Corba 3.4, Part 2, 7.10.2.4 CodeSet Component of IOR Multi-Component Profile
                                    // console.log(`IOR: component[${i}] = CODE_SETS`)
                                    break;
                                case TAG_POLICIES:
                                    // console.log(`IOR: component[${i}] = POLICIES`)
                                    break;
                                default:
                                    // console.log(`IOR: component[${i}] = ${id} (0x${id.toString(16)})`)
                            }
                            buffer.offset = nextOffset;
                        }
                    }
                } break;
                default:
                    cerr << format("IOR: Unhandled profile type={} {:x}", profileId, profileId) << endl;
            }
        });
    }
    return data;
}

}  // namespace CORBA