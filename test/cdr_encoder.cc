#include "../src/corba/cdr.hh"
#include <gtest/gtest.h>

TEST(CDRDEncoder, ULong) {
    CORBA::CDREncoder encoder;
    encoder.ulong(0xDEADBEEF);
    CORBA::CDRDecoder decoder(encoder);
    EXPECT_EQ(decoder.ulong(), 0xDEADBEEF);
}
