#pragma once

#include <stdexcept>
#include <string>
#include <opencv2/opencv.hpp>

class VideoWriter {
        std::string filename;

        cv::VideoWriter writer;
        bool isOpen = false;

    public:
        VideoWriter(const std::string_view &filename) : filename(filename) { }

        void frame(const cv::Mat &frame, double fps) {
            if (!isOpen) {
                // int codec = cv::VideoWriter::fourcc('M', 'J', 'P', 'G');
                // macos can't show this but opencv
                // int codec = cv::VideoWriter::fourcc('H', '2', '6', '4');
                // MPEG-4 Part 2
                // int codec = cv::VideoWriter::fourcc('M', 'P', '4', 'V');
                // MPEG-4 Part 10 aka. H.264
                int codec = cv::VideoWriter::fourcc('a', 'v', 'c', '1');

                bool isColor = true;
                writer.open(filename, codec, fps, frame.size(), isColor);
                if (!writer.isOpened()) {
                    throw std::runtime_error(format("failed to open {}", filename));
                }
                isOpen = true;
            }
            writer.write(frame);
        }
};
