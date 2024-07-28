#include "videosource.hh"

using namespace std;

VideoSource &VideoSource::operator>>(cv::Mat &image) {
    cap >> image;
    return *this;
}
