#include <bandit/bandit.h>

#include "../src/corba/cdr.hh"

using namespace snowhouse;
using namespace bandit;

using namespace std;

go_bandit([]() {
    describe("CDREncoder", []() {
        it("boolean()", []() {
            CORBA::CDREncoder encoder;
            encoder.boolean(0x42);
            CORBA::CDRDecoder decoder(encoder);
            AssertThat(decoder.octet(), Equals(0x01));
        });
        it("octet()", []() {
            CORBA::CDREncoder encoder;
            encoder.octet(0x42);
            CORBA::CDRDecoder decoder(encoder);
            AssertThat(decoder.octet(), Equals(0x42));
        });
        it("ushort()", []() {
            CORBA::CDREncoder encoder;
            encoder.ushort(0xDEAD);
            CORBA::CDRDecoder decoder(encoder);
            AssertThat(decoder.ushort(), Equals(0xDEAD));
        });
        it("ulong()", []() {
            CORBA::CDREncoder encoder;
            encoder.ulong(0xDEADBEEF);
            CORBA::CDRDecoder decoder(encoder);
            AssertThat(decoder.ulong(), Equals(0xDEADBEEF));
        });
        it("ulonglong()", []() {
            CORBA::CDREncoder encoder;
            encoder.ulonglong(0xDEADBEEFC0DEBABE);
            CORBA::CDRDecoder decoder(encoder);
            AssertThat(decoder.ulonglong(), Equals(0xDEADBEEFC0DEBABE));
        });
    });
});
