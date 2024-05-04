#pragma once

#include <string>
#include <vector>

class WavefrontObj {
    public:
        WavefrontObj(const std::string &filename);
        std::string filename;
        std::vector<float> xyz;
        std::vector<float> uv;
        std::vector<unsigned> vcount;
        std::vector<unsigned> fxyz;
        std::vector<unsigned> fuv;
};
