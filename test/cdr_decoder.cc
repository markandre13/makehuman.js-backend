#include "../src/corba/cdr.hh"
#include <gtest/gtest.h>

TEST(CDRDecoder, BooleanTrue) {
    CORBA::CDRDecoder data("\xFF", 1, std::endian::big);
    EXPECT_EQ(data.boolean(), true);
}

TEST(CDRDecoder, BooleanFalse) {
    CORBA::CDRDecoder data("\x00", 0, std::endian::big);
    EXPECT_EQ(data.boolean(), false);
}

TEST(CDRDecoder, Octet) {
    CORBA::CDRDecoder data("A", 1, std::endian::big);
    EXPECT_EQ(data.octet(), 'A');
}

TEST(CDRDecoder, Character) {
    CORBA::CDRDecoder data("A", 1, std::endian::big);
    EXPECT_EQ(data.character(), 'A');
}

TEST(CDRDecoder, UShortBigEndian) {
    CORBA::CDRDecoder data("..\xDE\xAD!", 5, std::endian::big);
    data.setOffset(2);
    EXPECT_EQ(data.ushort(), 0xDEAD);

    data.setOffset(1);
    EXPECT_EQ(data.ushort(), 0xDEAD);
    EXPECT_EQ(data.octet(), '!');
}

TEST(CDRDecoder, UShortLittleEndian) {
    CORBA::CDRDecoder data("\xAD\xDE", 2, std::endian::little);
    EXPECT_EQ(data.ushort(), 0xDEAD);
}

TEST(CDRDecoder, ULongBigEndian) {
    CORBA::CDRDecoder data("....\xDE\xAD\xBE\xEF!", 9, std::endian::big);

    data.setOffset(4);
    EXPECT_EQ(data.ulong(), 0xDEADBEEF);

    data.setOffset(1);
    EXPECT_EQ(data.ulong(), 0xDEADBEEF);
    EXPECT_EQ(data.octet(), '!');
}

TEST(CDRDecoder, ULongLittleEndian) {
    CORBA::CDRDecoder data("\xEF\xBE\xAD\xDE", 4, std::endian::little);
    EXPECT_EQ(data.ulong(), 0xDEADBEEF);
}

TEST(CDRDecoder, ULongLongBigEndian) {
    CORBA::CDRDecoder data("........\xDE\xAD\xBE\xEF\xC0\xDE\xBA\xBE!", 17, std::endian::big);

    data.setOffset(8);
    EXPECT_EQ(data.ulonglong(), 0xDEADBEEFC0DEBABE);

    data.setOffset(1);
    EXPECT_EQ(data.ulonglong(), 0xDEADBEEFC0DEBABE);
    EXPECT_EQ(data.octet(), '!');
}

TEST(CDRDecoder, ULongLongLittleEndian) {
    CORBA::CDRDecoder data("\xBE\xBA\xDE\xC0\xEF\xBE\xAD\xDE", 8, std::endian::little);
    EXPECT_EQ(data.ulonglong(), 0xDEADBEEFC0DEBABE);
}
