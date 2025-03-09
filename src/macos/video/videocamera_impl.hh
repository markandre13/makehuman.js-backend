#pragma once

#include "../../opencv/videocamera.hh"

class VideoCamera_impl : public VideoCamera2_skel {
    int _openCvIndex;
    std::string _id;
    std::string _name;
    std::string _features;

    double _fps;
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
    inline double fps() const { return _fps; }
    inline void fps(double fps) { _fps = fps; }

    CORBA::async<std::string> id() override;
    CORBA::async<std::string> name() override;
    CORBA::async<void> name(const std::string_view &) override;
    CORBA::async<std::string> features() override;
};

std::vector<std::shared_ptr<VideoCamera2>> getVideoCameras(std::shared_ptr<CORBA::ORB> orb);
