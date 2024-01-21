#include "../src/corba/cdr.hh"
#include <gtest/gtest.h>

#include <iostream>

using namespace std;

TEST(CDRDEncoder, Octet) {
    CORBA::CDREncoder encoder;
    encoder.octet(0x42);
    CORBA::CDRDecoder decoder(encoder);
    EXPECT_EQ(decoder.octet(), 0x42);
}

TEST(CDRDEncoder, UShort) {
    CORBA::CDREncoder encoder;
    encoder.ushort(0xDEAD);
    CORBA::CDRDecoder decoder(encoder);
    EXPECT_EQ(decoder.ushort(), 0xDEAD);
}

TEST(CDRDEncoder, ULong) {
    CORBA::CDREncoder encoder;
    encoder.ulong(0xDEADBEEF);
    CORBA::CDRDecoder decoder(encoder);
    EXPECT_EQ(decoder.ulong(), 0xDEADBEEF);
}

TEST(CDRDEncoder, ULongLong) {
    CORBA::CDREncoder encoder;
    encoder.ulonglong(0xDEADBEEFC0DEBABE);
    CORBA::CDRDecoder decoder(encoder);
    EXPECT_EQ(decoder.ulonglong(), 0xDEADBEEFC0DEBABE);
}