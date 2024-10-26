#pragma once

#include <fstream>
#include <string>
#include <stdexcept>
#include <print>

#include "../mediapipe/blazepose.hh"

/**
 * Read FreeMoCap body pose CSV file.
 */
class FreeMoCap {
        std::ifstream in;
        bool eof;

    public:
        FreeMoCap(const std::string &filename) : in(filename), eof(false) {
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
        inline bool isEof() {
            return eof;
        }

        /**
         * read the next pose.
         */
        void getPose(BlazePose *blazepose);
};