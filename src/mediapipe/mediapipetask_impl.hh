#pragma once

#include "../generated/makehuman_skel.hh"
#include <opencv2/opencv.hpp>

class Backend_impl;
class MediaPipeTask_impl: public MediaPipeTask_skel {
    protected:
        Backend_impl *_backend;

    public:
        MediaPipeTask_impl(Backend_impl *backend): _backend(backend) {}
        virtual void frame(const cv::Mat &frame, int64_t timestamp_ms) = 0;
};

std::vector<std::shared_ptr<MediaPipeTask>> getMediaPipeTasks(std::shared_ptr<CORBA::ORB> orb, Backend_impl *backend);
