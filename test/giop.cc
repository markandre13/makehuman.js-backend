#include "../src/corba/giop.hh"
#include "util.hh"
#include <gtest/gtest.h>

using namespace std;

TEST(GIOPDecoder, OmniOrbLocateRequest) {
    // OmniORB, IIOP 1.2, LocateRequest
    // 4749 4f50 0102 0103 2000 0000 0200 0000
    // ^         ^    ^  ^ ^         ^
    // |         |    |  | length 0x20 in little endian
    // |         |    |  message type
    // |         |    endian: 0 big, 1: little
    // |         version 1.2
    // "GIOP"

    auto data = parseOmniDump(R"(
        4749 4f50 0102 0103 2000 0000 0200 0000 GIOP.... .......
        0000 0000 1400 0000 ff62 6964 6972 fe97 .........bidir..
        c46b 6101 000f 5700 0000 0000           .ka...W.....)");
    CORBA::DataView dataview((char *)data.data(), data.size());
    CORBA::GIOPDecoder decoder(dataview);

    auto type = decoder.scanGIOPHeader();
    EXPECT_EQ(type, CORBA::LOCATE_REQUEST);
    EXPECT_EQ(decoder.type, CORBA::LOCATE_REQUEST);
    EXPECT_EQ(decoder.majorVersion, 1);
    EXPECT_EQ(decoder.minorVersion, 2);
    EXPECT_EQ(decoder.length + 12, data.size());
    EXPECT_EQ(dataview.getOffset(), 12);

    auto locateRequest = decoder.scanLocateRequest();
    EXPECT_EQ(locateRequest->requestId, 2);
    CORBA::CDRDecoder objectKey(dataview.data() + 24, 20);
    EXPECT_EQ(locateRequest->objectKey, objectKey);
}

TEST(GIOPDecoder, OmniOrbRequest) {
    auto data = parseOmniDump(R"(
        4749 4f50 0102 0100 e000 0000 0400 0000 GIOP............
        0300 0000 0000 0000 1400 0000 ff62 6964 .............bid
        6972 fe97 c46b 6101 000f 5700 0000 0000 ir...ka...W.....
        0b00 0000 7365 6e64 4f62 6a65 6374 0000 ....sendObject..
        0100 0000 0100 0000 0c00 0000 0100 0000 ................
        0100 0100 0901 0100 1200 0000 4944 4c3a ............IDL:
        4749 4f50 536d 616c 6c3a 312e 3000 0000 GIOPSmall:1.0...
        0100 0000 0000 0000 6800 0000 0101 0200 ........h.......
        0e00 0000 3139 322e 3136 382e 312e 3130 ....192.168.1.10
        3500 bbcf 1400 0000 ff62 6964 6972 fea8 5........bidir..
        c46b 6101 000f 7500 0000 0000 0200 0000 .ka...u.........
        0000 0000 0800 0000 0100 0000 0054 5441 .............TTA
        0100 0000 1c00 0000 0100 0000 0100 0100 ................
        0100 0000 0100 0105 0901 0100 0100 0000 ................
        0901 0100 0400 0000 666f 6f00           ........foo.)");
    CORBA::DataView dataview((char *)data.data(), data.size());
    CORBA::GIOPDecoder decoder(dataview);

    auto type = decoder.scanGIOPHeader();
    EXPECT_EQ(type, CORBA::REQUEST);
    EXPECT_EQ(decoder.type, CORBA::REQUEST);
    EXPECT_EQ(decoder.majorVersion, 1);
    EXPECT_EQ(decoder.minorVersion, 2);
    EXPECT_EQ(decoder.length + 12, data.size());
    EXPECT_EQ(dataview.getOffset(), 12);

    auto request = decoder.scanRequestHeader();
    EXPECT_TRUE(request->responseExpected);
    EXPECT_EQ(request->requestId, 4);
    CORBA::CDRDecoder objectKey(dataview.data() + 28, 20);
    EXPECT_EQ(request->objectKey, objectKey);
    EXPECT_EQ(request->method, string_view("sendObject"));
}

TEST(GIOPDecoder, OmniOrbReply) {
    auto data = parseOmniDump(R"(
        4749 4f50 0102 0101 0c00 0000 0400 0000 GIOP............
        0000 0000 0000 0000                     ........)");
    CORBA::DataView dataview((char *)data.data(), data.size());
    CORBA::GIOPDecoder decoder(dataview);

    auto type = decoder.scanGIOPHeader();
    EXPECT_EQ(type, CORBA::REPLY);
    EXPECT_EQ(decoder.type, CORBA::REPLY);
    EXPECT_EQ(decoder.majorVersion, 1);
    EXPECT_EQ(decoder.minorVersion, 2);
    EXPECT_EQ(decoder.length + 12, data.size());
    EXPECT_EQ(dataview.getOffset(), 12);

    // auto request = decoder.scanRequestHeader();
    // EXPECT_TRUE(request->responseExpected);
    // EXPECT_EQ(request->requestId, 4);
    // CORBA::DataView objectKey(dataview.data() + 28, 20);
    // EXPECT_EQ(request->objectKey, objectKey);
    // EXPECT_EQ(request->method, string_view("sendObject"));
}