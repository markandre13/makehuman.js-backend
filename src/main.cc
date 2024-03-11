#include <opencv2/opencv.hpp>
#include "gmod_api.h"
#include "../mediapipe/framework/formats/landmark.pb.h"

#include <print>

#include <span>

#include <corba/corba.hh>
#include <corba/net/ws.hh>

#include "makehuman_impl.hh"

using namespace std;

typedef const vector<::mediapipe::NormalizedLandmarkList> MultiFaceLandmarks;

// mediapipe        -> /Users/mark/upstream/mediapipe_cpp_lib/import_files
// mediapipe_graphs -> /Users/mark/upstream/mediapipe_cpp_lib/mediapipe_graphs

int main(void) {
    println("makehuman.js backend");

    //
    // SETUP ORB
    //
    auto orb = make_shared<CORBA::ORB>();
    auto backend = make_shared<Backend_impl>(orb);
    orb->bind("Backend", backend);

    struct ev_loop *loop = EV_DEFAULT;
    auto protocol = new CORBA::net::WsProtocol();
    protocol->listen(orb.get(), loop, "localhost", 9001);

    orb->registerProtocol(protocol);

    println("the audience is listening...");

    // http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#The_special_problem_of_fork:
    // To support fork in your child processes, you have to call ev_loop_fork () after
    // a fork in the child, enable EVFLAG_FORKCHECK, or resort to EVBACKEND_SELECT or
    // EVBACKEND_POLL.

    // ev_run(loop, 0);

    //
    // SETUP MEDIAPIPE
    //
    IGMOD *test = CreateGMOD();

    test->set_camera_props(0, 640, 480, 30);
    test->set_camera(true);
    test->set_overlay(true);

        // FOR GRAPHS THAT RETURN IMAGES
    // The current frame is stored as demo.png in the folder
    // This requires the graph to have ecoded_output_video ( or whatever stream name you put here ) as an output stream
    // Normally mediapipe graphs return mediapipe::ImageFrame as output, you can use the ImageFrameToOpenCVMatCalculator
    // to do convert it to a cv::Mat.
    //////////////////////////////////
    // auto obs = test->create_observer("encoded_output_video");
    // obs->SetPresenceCallback([](class IObserver* observer, bool present){});
    // obs->SetPacketCallback([](class IObserver* observer){
    //     string message_type = observer->GetMessageType();
    //     cout << message_type << endl;
    //     cv::Mat* test;
    //     test = static_cast<cv::Mat*>(observer->GetData());
    //     cv::imwrite("testing.png", *test);
    // });
    //////////////////////////////////

    // FOR GRAPH THAT RETURNS NORMALIZED LANDMARK LIST
    //////////////////////////////////
    // auto obs = test->create_observer("face_landmarks");
    // obs->SetPresenceCallback([](class IObserver* observer, bool present){});
    // obs->SetPacketCallback([](class IObserver* observer){
    //     string message_type = observer->GetMessageType();
    //     cout << message_type << endl;
    //     auto data = static_cast<mediapipe::NormalizedLandmarkList*>(observer->GetData());
    //     cout << data->landmark(0).x() << endl;
    // });
    //////////////////////////////////

    // FOR GRAPH THAT RETURNS MULTIPLE NORMALIZED LANDMARK LISTS
    //////////////////////////////////
    auto obs = test->create_observer("multi_face_landmarks");
    obs->SetPacketCallback([](class IObserver *observer) { cout << "packet" << endl; });
    obs->SetPresenceCallback([](class IObserver *observer, bool present) { cout << "present = " << (present ? "true" : "false") << endl; });
    obs->SetPacketCallback([&](class IObserver *observer) {
        // string message_type = observer->GetMessageType();
        // cout << message_type << endl;
        auto multi_face_landmarks = static_cast<MultiFaceLandmarks *>(observer->GetData());
        auto &lm = (*multi_face_landmarks)[0];

        // Landmark {float: x,y,z,visibility,presence }
        // cout << lm.landmark_size() << " landmarks: " << lm.landmark(0).x() << ", " << lm.landmark(0).x() << ", " << lm.landmark(0).z() << endl;
        // wsHandle();

        ev_run(loop, EVRUN_NOWAIT);

        // if (isFaceRequested()) {
            float float_array[lm.landmark_size() * 3];
            for (int i = 0; i < lm.landmark_size(); ++i) {
                float_array[i * 3] = lm.landmark(i).x();
                float_array[i * 3 + 1] = lm.landmark(i).y();
                float_array[i * 3 + 2] = lm.landmark(1).z();
            }
            std::span s { float_array, static_cast<size_t>(lm.landmark_size() * 3)};
            backend->mediapipe(s);
        //     sendFace(float_array, lm.landmark_size() * 3);
        // }
    });
    //////////////////////////////////

    // These are the graphs that have been tested and work

    // HOLISTIC TRACKING
    // test->start("mediapipe_graphs/holistic_tracking/holistic_tracking_cpu.pbtxt");
    // HAND TRACKING
    // test->start("mediapipe_graphs/hand_tracking/hand_detection_desktop_live.pbtxt");
    // test->start("mediapipe_graphs/hand_tracking/hand_tracking_desktop_live.pbtxt");
    // POSE TRACKING
    // test->start("mediapipe_graphs/pose_tracking/pose_tracking_cpu.pbtxt");
    // IRIS TRACKING
    // test->start("mediapipe_graphs/iris_tracking/iris_tracking_cpu.pbtxt");
    // test->start("mediapipe_graphs/iris_tracking/iris_tracking_gpu.pbtxt");
    // FACE DETECTION
    // test->start("mediapipe_graphs/face_detection/face_detection_desktop_live.pbtxt");
    // test->start("mediapipe_graphs/face_detection/face_detection_full_range_desktop_live.pbtxt");
    // FACE MESH
    test->start("mediapipe_graphs/face_mesh/face_mesh_desktop_live.pbtxt");
    // SELFIE SEGMENTATION
    // test->start("mediapipe_graphs/selfie_segmentation/selfie_segmentation_cpu.pbtxt");
    // HAIR SEGMENTATION
    // test->start("mediapipe_graphs/hair_segmentation/hair_segmentation_desktop_live.pbtxt");

    return 0;
}
