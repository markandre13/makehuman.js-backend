#include <bandit/bandit.h>

#include "../src/corba/cdr.hh"

using namespace snowhouse;
using namespace bandit;

using namespace std;

go_bandit([]() {
    describe("CDRDecoder", []() {
        describe("boolean()", []() {
            it("true", []() {
                CORBA::CDRDecoder data("\xFF", 1, std::endian::big);
                AssertThat(data.boolean(), Equals(true));
            });
            it("false", []() {
                CORBA::CDRDecoder data("\x00", 0, std::endian::big);
                AssertThat(data.boolean(), Equals(false));
            });
        });
        it("octet()", []() {
            CORBA::CDRDecoder data("A", 1, std::endian::big);
            AssertThat(data.octet(), Equals('A'));
        });
        it("character()", []() {
            CORBA::CDRDecoder data("A", 1, std::endian::big);
            AssertThat((char)data.character(), Equals('A')); // FIXME: snowhouse???
        });
        describe("ushort()", []() {
            it("big endian()", []() {
                CORBA::CDRDecoder data("..\xDE\xAD!", 5, std::endian::big);
                data.setOffset(2);
                AssertThat(data.ushort(), Equals(0xDEAD));

                data.setOffset(1);
                AssertThat(data.ushort(), Equals(0xDEAD));
                AssertThat(data.octet(), Equals('!'));
            });
            it("little endian()", []() {
                CORBA::CDRDecoder data("\xAD\xDE", 2, std::endian::little);
                AssertThat(data.ushort(), Equals(0xDEAD));
            });
        });
        describe("ulong()", []() {
            it("big endian()", []() {
                CORBA::CDRDecoder data("....\xDE\xAD\xBE\xEF!", 9, std::endian::big);
                data.setOffset(4);
                AssertThat(data.ulong(), Equals(0xDEADBEEF));

                data.setOffset(1);
                AssertThat(data.ulong(), Equals(0xDEADBEEF));
                AssertThat(data.octet(), Equals('!'));
            });
            it("little endian()", []() {
                CORBA::CDRDecoder data("\xEF\xBE\xAD\xDE", 4, std::endian::little);
                AssertThat(data.ulong(), Equals(0xDEADBEEF));
            });
        });
        describe("ulonglong()", []() {
            it("big endian()", []() {
                CORBA::CDRDecoder data("........\xDE\xAD\xBE\xEF\xC0\xDE\xBA\xBE!", 17, std::endian::big);
                data.setOffset(8);
                AssertThat(data.ulonglong(), Equals(0xDEADBEEFC0DEBABE));

                data.setOffset(1);
                AssertThat(data.ulonglong(), Equals(0xDEADBEEFC0DEBABE));
                AssertThat(data.octet(), Equals('!'));
            });
            it("little endian()", []() {
                CORBA::CDRDecoder data("\xBE\xBA\xDE\xC0\xEF\xBE\xAD\xDE", 8, std::endian::little);
                AssertThat(data.ulonglong(), Equals(0xDEADBEEFC0DEBABE));
            });
        });
    });
});
