#include "../src/corba/cdr.hh"
#include "kaffeeklatsch.hh"

using namespace kaffeeklatsch;

kaffeeklatsch_spec([] {
    describe("CDRDecoder", [] {
        describe("boolean()", [] {
            it("true", [] {
                CORBA::CDRDecoder data("\xFF", 1, std::endian::big);
                expect(data.boolean()).equals(true);
            });
            it("false", [] {
                CORBA::CDRDecoder data("\x00", 0, std::endian::big);
                expect(data.boolean()).equals(false);
            });
        });
        it("octet()", [] {
            CORBA::CDRDecoder data("A", 1, std::endian::big);
            expect(data.octet()).equals('A');
        });
        it("character()", [] {
            CORBA::CDRDecoder data("A", 1, std::endian::big);
            expect((char)data.character()).equals('A');  // FIXME: type cast
        });
        describe("ushort()", [] {
            it("big endian()", [] {
                CORBA::CDRDecoder data("..\xDE\xAD!", 5, std::endian::big);
                data.setOffset(2);
                expect(data.ushort()).equals(0xDEAD);

                data.setOffset(1);
                expect(data.ushort()).equals(0xDEAD);
                expect(data.octet()).equals('!');
            });
            it("little endian()", [] {
                CORBA::CDRDecoder data("\xAD\xDE", 2, std::endian::little);
                expect(data.ushort()).equals(0xDEAD);
            });
        });
        describe("ulong()", [] {
            it("big endian()", []() {
                CORBA::CDRDecoder data("....\xDE\xAD\xBE\xEF!", 9, std::endian::big);
                data.setOffset(4);
                expect(data.ulong()).equals(0xDEADBEEF);

                data.setOffset(1);
                expect(data.ulong()).equals(0xDEADBEEF);
                expect(data.octet()).equals('!');
            });
            it("little endian()", [] {
                CORBA::CDRDecoder data("\xEF\xBE\xAD\xDE", 4, std::endian::little);
                expect(data.ulong()).equals(0xDEADBEEF);
            });
        });
        describe("ulonglong()", [] {
            it("big endian()", []() {
                CORBA::CDRDecoder data("........\xDE\xAD\xBE\xEF\xC0\xDE\xBA\xBE!", 17, std::endian::big);
                data.setOffset(8);
                expect(data.ulonglong()).equals(0xDEADBEEFC0DEBABE);

                data.setOffset(1);
                expect(data.ulonglong()).equals(0xDEADBEEFC0DEBABE);
                expect(data.octet()).equals('!');
            });
            it("little endian()", [] {
                CORBA::CDRDecoder data("\xBE\xBA\xDE\xC0\xEF\xBE\xAD\xDE", 8, std::endian::little);
                expect(data.ulonglong()).equals(0xDEADBEEFC0DEBABE);
            });
        });
    });
});
