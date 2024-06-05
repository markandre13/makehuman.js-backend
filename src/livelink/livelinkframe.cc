#include "livelinkframe.hh"
#include <print>
#include <stdexcept>

using namespace std;

LiveLinkFrame::LiveLinkFrame(unsigned char *data, size_t nbytes): data(data), nbytes(nbytes) {
    packetVersion = getByte();
    if (packetVersion != 6) {
        throw runtime_error(format("LiveLinkFrame: got packet version {} but expected 6", packetVersion));
    }
    deviceId = getString();
    subjectName = getString();
    frame = getDWord();
    subframe = getDWord();
    fps = getDWord();
    fpsDenominator = getDWord();
    blendshapeCount = getByte();
    if (blendshapeCount != 61) {
         throw runtime_error(format("LiveLinkFrame: got {} weights but expected 61", packetVersion));
    }
    memcpy(weights, data+offset, 61 * sizeof(float));
    uint32_t *ptr = reinterpret_cast<uint32_t*>(weights);
    for(uint8_t i=0; i<61; ++i, ++ptr) {
        *ptr = __builtin_bswap32(*ptr);
    }
}

uint8_t LiveLinkFrame::getByte() {
    return data[offset++];
}

uint32_t LiveLinkFrame::getDWord() {
    uint32_t result = data[offset++];
    result <<= 8;
    result |= data[offset++];
    result <<= 8;
    result |= data[offset++];
    result <<= 8;
    result |= data[offset++];
    return result;
}

float LiveLinkFrame::getFloat() {
    union {
        unsigned char d[4];
        float f;
    } r;
    r.d[3] = data[offset++];
    r.d[2] = data[offset++];
    r.d[1] = data[offset++];
    r.d[0] = data[offset++];
    return r.f;
}

string_view LiveLinkFrame::getString() {
    uint32_t len = getDWord();
    string_view str(reinterpret_cast<const char*>(data + offset), len);
    offset += len;
    return str;
}

std::vector<std::string_view> LiveLinkFrame::blendshapeNames {
    "eyeBlinkLeft", // 0
    "eyeLookDownLeft", // 1
    "eyeLookInLeft", // 2
    "eyeLookOutLeft", // 3
    "eyeLookUpLeft", // 4
    "eyeSquintLeft", // 5
    "eyeWideLeft", // 6
    "eyeBlinkRight", // 7
    "eyeLookDownRight", // 8
    "eyeLookInRight", // 9
    "eyeLookOutRight", // 10
    "eyeLookUpRight", // 11
    "eyeSquintRight", // 12
    "eyeWideRight", // 13
    "jawForward", // 14
    "jawLeft", // 15
    "jawRight", // 16
    "jawOpen", // 17
    "mouthClose", // 18
    "mouthFunnel", // 19
    "mouthPucker", // 20
    "mouthLeft", // 21
    "mouthRight", // 22
    "mouthSmileLeft", // 23
    "mouthSmileRight", // 24
    "mouthFrownLeft", // 25
    "mouthFrownRight", // 26
    "mouthDimpleLeft", // 27
    "mouthDimpleRight", // 28
    "mouthStretchLeft", // 29
    "mouthStretchRight", // 30
    "mouthRollLower", // 31
    "mouthRollUpper", // 32
    "mouthShrugLower", // 33
    "mouthShrugUpper", // 34
    "mouthPressLeft", // 35
    "mouthPressRight", // 36
    "mouthLowerDownLeft", // 37
    "mouthLowerDownRight", // 38
    "mouthUpperUpLeft", // 39
    "mouthUpperUpRight", // 40
    "browDownLeft", // 41
    "browDownRight", // 42
    "browInnerUp", // 43
    "browOuterUpLeft", // 44
    "browOuterUpRight", // 45
    "cheekPuff", // 46
    "cheekSquintLeft", // 47
    "cheekSquintRight", // 48
    "noseSneerLeft", // 49
    "noseSneerRight", // 50
    // extra, https://simple.wikipedia.org/wiki/Pitch,_yaw,_and_roll
    "tongueOut", // 51
    "headYaw", // 52
    "headPitch", // 53
    "headRoll", // 54
    "leftEyeYaw", // 55
    "leftEyePitch", // 56
    "leftEyeRoll", // 57
    "rightEyeYaw", // 58
    "rightEyePitch", // 59
    "rightEyeRoll", // 60
};
