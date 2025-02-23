#pragma once

#include <cc_lib/mediapipe.hh>

#include "mediapipetask_impl.hh"

class MediapipePose : public MediaPipeTask_impl {
        std::unique_ptr<mediapipe::cc_lib::vision::pose_landmarker::PoseLandmarker> mp;

    public:
        MediapipePose(Backend_impl*);
        CORBA::async<std::string> name() override;

        void frame(const cv::Mat &frame, int64_t timestamp_ms) override;
};
