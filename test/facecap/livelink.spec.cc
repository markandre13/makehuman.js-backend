#include "../src/facecap/livelinkframe.hh"
#include "kaffeeklatsch.hh"

#include <string>
#include <print>

using namespace kaffeeklatsch;
using namespace std;

vector<uint8_t> parseHexdump(const string_view &data);


// https://forums.unrealengine.com/t/unreal-live-link-udp-packet-format/472878/2
// on UE5 (PacketVersion 6) its
// uint8 - packet version (6)
// uint32 - device id length
// char - device id
// uint32 - subject name length
// char - subject name
// uint32 - Frame
// uint32 - SubFrame
// uint32 - fps
// uint32 - denominator
// uint8 - number of blendshapes (must be 61)
// float - the 61 blend shapes

kaffeeklatsch_spec([] {
    describe("class LiveLink", [] {
        it("class LiveLinkFrame", [] {
            auto dump = parseHexdump(R"(
                0000 06 00 00 00 24 35 39 30 34 41 41 34 42 2d 41 44 ....$5904AA4B-AD
                0010 39 44 2d 34 33 45 35 2d 39 38 41 42 2d 38 39 41 9D-43E5-98AB-89A
                0020 41 45 42 38 41 42 32 37 37 00 00 00 06 69 50 68 AEB8AB277....iPh
                0030 6f 6e 65 00 0a 3f d9 3f 07 02 80 00 00 00 3c 00 one..?.?......<.
                0040 00 00 01 3d 00 00 00 00 00 00 00 00 3e c9 b8 6a ...=........>..j
                0050 00 00 00 00 3e d7 96 e6 3e b5 13 eb 3e b3 47 98 ....>...>...>.G.
                0060 00 00 00 00 00 00 00 00 00 00 00 00 3e 97 ab 08 ............>...
                0070 3e da 44 3d 3e b5 0f 2f 3e b3 3f 63 3c e3 c7 5e >.D=>../>.?c<..^
                0080 3b b0 00 94 00 00 00 00 3d 40 80 09 3d 48 0c 85 ;.......=@..=H..
                0090 3d 59 1b 19 3d a4 59 92 3d 09 46 e1 00 00 00 00 =Y..=.Y.=.F.....
                00a0 3d a7 f2 d7 3d 51 d7 20 00 00 00 00 00 00 00 00 =...=Q. ........
                00b0 3d 62 4d 76 3d 43 58 68 3e 3e 4e 89 3e 30 ca d7 =bMv=CXh>>N.>0..
                00c0 3d b9 7b 00 3c d0 49 06 3e 06 f9 f0 3d a6 c3 de =.{.<.I.>...=...
                00d0 3d a9 2d 8f 3d af c4 7d 3d 87 9d 09 3d 6e e3 a3 =.-.=..}=...=n..
                00e0 3c ab aa a7 3c 85 38 1d 3e 8a 65 4d 3e 8a 65 8d <...<.8.>.eM>.e.
                00f0 3d de e4 e4 00 00 00 00 00 00 00 00 3c d7 3e e1 =...........<.>.
                0100 3d fa 6c e0 3d e0 ae af 3d f9 90 0b 3d bf 04 08 =.l.=...=...=...
                0110 32 5e 53 9a bd 39 d0 a6 3e 62 75 a2 3d a6 2c 00 2^S..9..>bu.=.,.
                0120 3e 36 11 16 be 41 8f 99 bd 0a 89 a9 3e 72 38 60 >6...A......>r8`
                0130 be 41 97 25 bd 37 89 78                         .A.%.7.x        )");
            LiveLinkFrame frame(dump.data(), dump.size());
            expect(frame.deviceId).to.equal("5904AA4B-AD9D-43E5-98AB-89AAEB8AB277");
            expect(frame.subjectName).to.equal("iPhone");
            expect(frame.frame).to.equal(671705);
            expect(frame.subframe).to.equal(1057424000);
            expect(frame.fps).to.equal(60);
            expect(frame.fpsDenominator).to.equal(1);
            expect(frame.weights[0]).to.equal(0);
            expect(frame.weights[2]).to.equal(0.3939851);
            expect(frame.weights[60]).to.equal(-0.044808835);
        });
    });
});

inline unsigned char x2n(char c) {
    if ('0' <= c && c <= '9') {
        return c - '0';
    }
    if ('a' <= c && c <= 'f') {
        return c - 'a' + 10;
    }
    if ('A' <= c && c <= 'F') {
        return c - 'A' + 10;
    }
    throw runtime_error(format("x2n('{}'): not a hexadecimal digit", c));
}

vector<uint8_t> parseHexdump(const string_view &data) {
    int state = 0;
    vector<uint8_t> result;
    unsigned char b;
    for(auto &c: data) {
        // println("{} '{}' {}", state, c, result.size());
        switch(state) {
            case 0: // wait for line
                if (isxdigit(c)) {
                    state = 1;
                }
                break;
            case 1: // skip address
                if (!isxdigit(c)) {
                    if (c != ' ') {
                        throw runtime_error("expected ' ' after address");
                    }
                    state = 2;
                }
                break;
            case 2: // MSB
                if (!isxdigit(c)) {
                    state = -1;
                } else {
                    b = x2n(c) << 4;
                    state = 3;
                }
                break;
            case 3: // LSB
                b |= x2n(c);
                result.push_back(b);
                if ((result.size() & 0xF) == 0) {
                    state = 5;
                } else {
                    state = 4;
                }
                break;
            case 4: // space after byte
                if (c != ' ') {
                    throw runtime_error("expected ' ' after byte");
                } 
                state = 2;
                break;
            case 5: // skip till end of line
                if (c == '\n') {
                    state = 0;
                }
                break;
        }
    }
    return result;
}
