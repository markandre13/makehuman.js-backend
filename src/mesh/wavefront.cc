#include "wavefront.hh"

#include <string>
#include <sstream>
#include <print>
#include <vector>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "../util.hh"

using namespace std;

inline float stof(const std::string_view &s) {
    return strtof(s.data(), nullptr);
}

inline unsigned stou(const std::string_view &s) {
    return strtoul(s.data(), nullptr, 10);
}

WavefrontObj::WavefrontObj(const std::string &filename) {
    this->filename = filename;

    int fd = open(filename.data(), O_RDONLY);
    if (fd < 0) {
        throw runtime_error(format("failed to open file '{}': {}", filename, strerror(errno)));
    }
    off_t len = lseek(fd, 0, SEEK_END);
    if (len < 0) {
        throw runtime_error(format("failed to get size of file '{}': {}", filename, strerror(errno)));
    }
    const char * data = (const char*)mmap(nullptr, len, PROT_READ, MAP_FILE | MAP_PRIVATE, fd, 0);
    if (data == MAP_FAILED) {
        throw runtime_error(format("failed to mmap file '{}': {}", filename, strerror(errno)));
    }

    unsigned lineNumber = 0;
    for(auto line: split(string_view(data, len))) {
        ++lineNumber;
        line = trim(line);
        if (line.size() == 0) {
            continue;
        }
        if (line[0] == '#') {
            continue;
        }
        auto token = split(line, ' ');
        if (token[0] == "v") { // vertex X Y Z [W]
            if (token.size() != 4) {
                throw runtime_error(format("{}:{}: vertex (v) must have 3 arguments got {}", filename, lineNumber, token.size()-1));
            }
            xyz.push_back(stof(token[1]));
            xyz.push_back(stof(token[2]));
            xyz.push_back(stof(token[3]));
            continue;
        }       
        if (token[0] == "vt") { // vertex texture U V
            if (token.size() != 3) {
                throw runtime_error(format("{}:{}: vertex texture (vt) must have 2 arguments got {}", filename, lineNumber, token.size()-1));
            }
            uv.push_back(stof(token[1]));
            uv.push_back(stof(token[2]));
            continue;
        }
        if (token[0] == "f") {
            vcount.push_back(token.size() - 1);
            for(size_t i=1; i<token.size(); ++i) {
                auto edge = split(token[i], '/');
                if (edge.size() >= 1) {
                    fxyz.push_back(stou(edge[0]) - 1);
                }
                if (edge.size() >= 2) {
                    fuv.push_back(stou(edge[1]) - 1);
                }
            }
        }
    }

    munmap((void*)data, len);
    close(fd);
}
