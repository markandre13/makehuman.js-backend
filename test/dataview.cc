#include "../src/corba/dataview.hh"
#include <gtest/gtest.h>

TEST(DataView, BooleanTrue) {
    CORBA::DataView data("\xFF", 1, std::endian::big);
    EXPECT_EQ(data.boolean(), true);
}

TEST(DataView, BooleanFalse) {
    CORBA::DataView data("\x00", 0, std::endian::big);
    EXPECT_EQ(data.boolean(), false);
}

TEST(DataView, Octet) {
    CORBA::DataView data("A", 1, std::endian::big);
    EXPECT_EQ(data.octet(), 'A');
}

TEST(DataView, Character) {
    CORBA::DataView data("A", 1, std::endian::big);
    EXPECT_EQ(data.character(), 'A');
}

TEST(DataView, UShortBigEndian) {
    CORBA::DataView data("..\xDE\xAD!", 5, std::endian::big);
    data.setOffset(2);
    EXPECT_EQ(data.ushort(), 0xDEAD);

    data.setOffset(1);
    EXPECT_EQ(data.ushort(), 0xDEAD);
    EXPECT_EQ(data.octet(), '!');
}

TEST(DataView, UShortLittleEndian) {
    CORBA::DataView data("\xAD\xDE", 2, std::endian::little);
    EXPECT_EQ(data.ushort(), 0xDEAD);
}

TEST(DataView, ULongBigEndian) {
    CORBA::DataView data("....\xDE\xAD\xBE\xEF!", 9, std::endian::big);

    data.setOffset(4);
    EXPECT_EQ(data.ulong(), 0xDEADBEEF);

    data.setOffset(1);
    EXPECT_EQ(data.ulong(), 0xDEADBEEF);
    EXPECT_EQ(data.octet(), '!');
}

TEST(DataView, ULongLittleEndian) {
    CORBA::DataView data("\xEF\xBE\xAD\xDE", 4, std::endian::little);
    EXPECT_EQ(data.ulong(), 0xDEADBEEF);
}

TEST(DataView, ULongLongBigEndian) {
    CORBA::DataView data("........\xDE\xAD\xBE\xEF\xC0\xDE\xBA\xBE!", 17, std::endian::big);

    data.setOffset(8);
    EXPECT_EQ(data.ulonglong(), 0xDEADBEEFC0DEBABE);

    data.setOffset(1);
    EXPECT_EQ(data.ulonglong(), 0xDEADBEEFC0DEBABE);
    EXPECT_EQ(data.octet(), '!');
}

TEST(DataView, ULongLongLittleEndian) {
    CORBA::DataView data("\xBE\xBA\xDE\xC0\xEF\xBE\xAD\xDE", 8, std::endian::little);
    EXPECT_EQ(data.ulonglong(), 0xDEADBEEFC0DEBABE);
}
