#pragma once

#include "../generated/makehuman_skel.hh"
#include "videosource.hh"

class VideoCamera_impl : public VideoCamera2_skel {
    int _openCvIndex;
    std::string _id;
    std::string _name;
    std::string _features;
public:
    /**
     * Represent one of the video cameras connected to the computer.
     * 
     * \param openCvIndex Index used by OpenCV to identify the camera.
     * \param id          The camera's hardware id.
     * \param name        The camera's name
     * \param features    Description of the cameras features.
     */
    VideoCamera_impl(int openCvIndex, std::string_view &id, std::string_view name, const std::string &features): _openCvIndex(openCvIndex), _id(id), _name(name), _features(features) {}
    inline int openCvIndex() const { return _openCvIndex; }
    CORBA::async<std::string> id() override;
    CORBA::async<std::string> name() override;
    CORBA::async<void> name(const std::string_view &) override;
    CORBA::async<std::string> features() override;
};

/**
 * \deprecated By VideoCamera(2) in IDL
 */
class VideoCamera : public VideoSource {
    private:
        cv::VideoCapture cap;
        int deviceID;
        int apiID;

    public:
        VideoCamera(int deviceID = 0, int apiID = cv::CAP_ANY) : deviceID(deviceID), apiID(apiID) { open(); }
        VideoSource &operator>>(cv::Mat &image) override;
        double fps() override;
        void reset() override;
        int delay() const override;

    private:
        void open();
};
