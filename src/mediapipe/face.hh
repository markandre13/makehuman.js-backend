#pragma once

#include "mediapipetask_impl.hh"
#include <cc_lib/mediapipe.hh>

class MediapipeFace : public MediaPipeTask_impl {
        std::unique_ptr<mediapipe::cc_lib::vision::face_landmarker::FaceLandmarker> mp;
        bool blendshapeNamesHaveBeenSend = false;
    public:
        MediapipeFace(Backend_impl*);
        CORBA::async<std::string> name() override;
        void frame(const cv::Mat &frame, int64_t timestamp_ms) override;
};
