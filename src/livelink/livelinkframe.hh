#pragma once

#include <string>
#include <vector>
#include <cstdint>

class LiveLinkFrame {
    public:
        LiveLinkFrame(unsigned char *data, size_t nbytes);

        uint8_t packetVersion;
        std::string_view deviceId;
        std::string_view subjectName;
        uint32_t frame;
        uint32_t subframe;
        uint32_t fps;
        uint32_t fpsDenominator;
        uint8_t blendshapeCount;
        static std::vector<std::string_view> blendshapeNames;
        float weights[61];

    protected:
        size_t offset = 0;
        unsigned char *data;
        size_t nbytes;

        uint8_t getByte();
        uint32_t getDWord();
        std::string_view getString();
        float getFloat();
};