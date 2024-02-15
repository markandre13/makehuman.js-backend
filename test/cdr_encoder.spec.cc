#include "../src/corba/cdr.hh"
#include "kaffeeklatsch.hh"

using namespace kaffeeklatsch;

kaffeeklatsch_spec([] {
    describe("CDREncoder", [] {
        it("boolean()", [] {
            CORBA::CDREncoder encoder;
            encoder.boolean(0x42);
            CORBA::CDRDecoder decoder(encoder);
            expect(decoder.octet()).equals(0x01);
        });
        it("octet()", [] {
            CORBA::CDREncoder encoder;
            encoder.octet(0x42);
            CORBA::CDRDecoder decoder(encoder);
            expect(decoder.octet()).equals(0x42);
        });
        it("ushort()", [] {
            CORBA::CDREncoder encoder;
            encoder.ushort(0xDEAD);
            CORBA::CDRDecoder decoder(encoder);
            expect(decoder.ushort()).equals(0xDEAD);
        });
        it("ulong()", [] {
            CORBA::CDREncoder encoder;
            encoder.ulong(0xDEADBEEF);
            CORBA::CDRDecoder decoder(encoder);
            expect(decoder.ulong()).equals(0xDEADBEEF);
        });
        it("ulonglong()", [] {
            CORBA::CDREncoder encoder;
            encoder.ulonglong(0xDEADBEEFC0DEBABE);
            CORBA::CDRDecoder decoder(encoder);
            expect(decoder.ulonglong()).equals(0xDEADBEEFC0DEBABE);
        });
    });
});
