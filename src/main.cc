#include <opencv2/opencv.hpp>

// #include "gmod_api.h"
// #include "../mediapipe/framework/formats/landmark.pb.h"

#include <cc_lib/mediapipe.hh>
using mediapipe::cc_lib::vision::core::RunningMode;
using mediapipe::cc_lib::vision::face_landmarker::FaceLandmarkerOptions;
using mediapipe::cc_lib::vision::face_landmarker::FaceLandmarker;

#include <corba/corba.hh>
#include <corba/net/tcp.hh>
#include <corba/net/ws.hh>

#include <iostream>
#include <print>
#include <span>

#include "makehuman_impl.hh"

using namespace std;

// typedef const vector<::mediapipe::NormalizedLandmarkList> MultiFaceLandmarks;

// mediapipe        -> /Users/mark/upstream/mediapipe_cpp_lib/import_files
// mediapipe_graphs -> /Users/mark/upstream/mediapipe_cpp_lib/mediapipe_graphs

class Server_impl : public Server_skel {
        std::shared_ptr<Frontend> frontend;

    public:
        Server_impl(std::shared_ptr<CORBA::ORB> orb) : Server_skel(orb) {}
        CORBA::async<void> hello() override {
            println("HELLO");
            co_return;
        }
};

int main(void) {
    println("makehuman.js backend");

    //
    // SETUP ORB
    //
    auto orb = make_shared<CORBA::ORB>();
    orb->debug = true;

    struct ev_loop *loop = EV_DEFAULT;
    println("the audience is listening...");
#if 0
    auto server = make_shared<Server_impl>(orb);
    orb->bind("Server", server);

    auto protocol = new CORBA::net::TcpProtocol();
    // TODO: support 0.0.0.0, etc.
    protocol->listen(orb.get(), loop, "192.168.178.24", 9001);
    orb->registerProtocol(protocol);


    // http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#The_special_problem_of_fork:
    // To support fork in your child processes, you have to call ev_loop_fork () after
    // a fork in the child, enable EVFLAG_FORKCHECK, or resort to EVBACKEND_SELECT or
    // EVBACKEND_POLL.

    ev_run(loop, 0);
#else
    auto backend = make_shared<Backend_impl>(orb);
    orb->bind("Backend", backend);

    auto protocol = new CORBA::net::WsProtocol();
    protocol->listen(orb.get(), loop, "localhost", 9001);

    //
    // SETUP MEDIAPIPE
    //
#if 1
    auto options = std::make_unique<FaceLandmarkerOptions>();
    options->base_options.model_asset_path = "/Users/mark/python/py311-venv-mediapipe/face_landmarker_v2_with_blendshapes.task";
    options->running_mode = RunningMode::IMAGE;
    options->output_face_blendshapes = true;
    options->output_facial_transformation_matrixes = true;
    options->num_faces = 1;
    auto landmarker = FaceLandmarker::Create(std::move(options));

    cv::Mat input = cv::imread("upstream/mediapipe_cc_lib/mediapipe/objc/testdata/sergey.png");
    if (input.data == nullptr) {
        cerr << "failed to load image" << endl;
        return 1;
    }

    auto result = landmarker->Detect(input.channels(), input.cols, input.rows, input.step, input.data);
    if (!result.has_value()) {
        cerr << "failed to detect face landmarks" << endl;
        return 1;
    }

    if (result->face_landmarks.size() != 1) {
        cerr << "expect one face" << endl;
        return 1;
    }
    if (result->face_landmarks[0].landmarks.size() != 478) {
        cerr << "expected 478 landmarks for face, found " << result->face_landmarks[0].landmarks.size() << endl;
        return 1;
    }
    auto &lm = result->face_landmarks[0].landmarks[0];
    if (fabs(lm.x - 0.494063) > 0.000001) {
        cerr << "expected x = 0.494063, got " << lm.x << endl;
        return 1;
    }
    if (fabs(lm.y - 0.646164) > 0.000001) {
        cerr << "expected x = 0.646164, got " << lm.x << endl;
        return 1;
    }
    if (fabs(lm.z - -0.06861) > 0.000001) {
        cerr << "expected x = -0.06861, got " << lm.x << endl;
        return 1;
    }

    cout << "OKAY" << endl;

#else
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
    // obs->SetPacketCallback([](class IObserver *observer) { cout << "packet" << endl; });
    // obs->SetPresenceCallback([](class IObserver *observer, bool present) { cout << "present = " << (present ? "true" : "false") << endl; });
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
        float *ptr = float_array;
        for (int i = 0; i < lm.landmark_size(); ++i) {
            *(ptr++) = lm.landmark(i).x();
            *(ptr++) = lm.landmark(i).y();
            *(ptr++) = lm.landmark(i).z();
        }
        std::span s{float_array, static_cast<size_t>(lm.landmark_size() * 3)};
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
    // face_mesh_desktop.pbtxt takes an input file and writes an output file.
    // for production: record the video, analyze later. this way we get precission and the don't loose performances.
    // for development, etc.: live

    // SELFIE SEGMENTATION
    // test->start("mediapipe_graphs/selfie_segmentation/selfie_segmentation_cpu.pbtxt");
    // HAIR SEGMENTATION
    // test->start("mediapipe_graphs/hair_segmentation/hair_segmentation_desktop_live.pbtxt");

    // there is a test->stop()
#endif

#endif
    return 0;
}
