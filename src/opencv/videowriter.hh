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
                int codec = cv::VideoWriter::fourcc('M', 'J', 'P', 'G');
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
