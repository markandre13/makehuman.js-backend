#pragma once

#include <fstream>
#include <print>
#include <stdexcept>
#include <string>
#include <vector>

#include "../capturedevice.hh"
#include "../mediapipe/blazepose.hh"

/**
 * Read FreeMoCap body pose CSV file.
 */
class FreeMoCap {
        std::ifstream in;
        bool eof;

    public:
        FreeMoCap(const std::string& filename) : in(filename), eof(false) {
            if (!in) {
                throw std::runtime_error(format("failed to open file '{}'", filename));
            }
            // skip csv header
            std::string line;
            getline(in, line);
            // println("got line '{}'", line);
        }

        /**
         * 'true' when the last frame has been read.
         */
        inline bool isEof() const { return eof; }

        /**
         * read the next pose.
         */
        void getPose(BlazePose* blazepose);
};

/**
 * Read FreeMoCap into memory for random access.
 */
class MoCap {
        std::vector<BlazePose> store;

    public:
        MoCap(FreeMoCap&& mocap);
        size_t size() const { return store.size(); }
        const BlazePose& operator[](size_t pos) { return store[pos]; }
};
