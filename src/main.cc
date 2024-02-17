import std;

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

// #include <format>
// #include <iostream>
#include <opencv2/opencv.hpp>
// #include <vector>

#include "../mediapipe/framework/formats/landmark.pb.h"
#include "gmod_api.h"

// #include "corba/ws/EventHandler.hh"
#include "corba/corba.hh"
#include "corba/orb.hh"
#include "corba/giop.hh"

using namespace std;
using std::string, std::vector, std::cout, std::endl;

class Backend_skel : public CORBA::Skeleton {
    public:
        Backend_skel(CORBA::ORB *orb) : Skeleton(orb) {}
};

class Backend_impl : public Backend_skel {
    public:
        Backend_impl(CORBA::ORB *orb) : Backend_skel(orb) {}

    protected:
        void _call(const std::string_view &operation, CORBA::GIOPDecoder &decoder, CORBA::GIOPEncoder &encoder);
};

void Backend_impl::_call(const std::string_view &operation, CORBA::GIOPDecoder &decoder, CORBA::GIOPEncoder &encoder) {}

void chordataLoop();
static void mediapipeLoop();

bool isFaceRequested();
void sendFace(float *float_array, int size);

typedef const vector<::mediapipe::NormalizedLandmarkList> MultiFaceLandmarks;

// struct WsProtocol : public CORBA::detail::Protocol {
//         CORBA::task<CORBA::detail::Connection *> connect(const CORBA::ORB *orb, const std::string &hostname, uint16_t port);
//         CORBA::task<void> close();

//         static void *buffer;
//         static size_t size;
// };

// class WsConnection : public CORBA::detail::Connection {
//         std::string m_localAddress;
//         uint16_t m_localPort;
//         std::string m_remoteAddress;
//         uint16_t m_remotePort;

//     public:
//         WsConnection(const string &remoteAddress, uint16_t remotePort)
//             : m_localAddress("localhost"), m_localPort(1111), m_remoteAddress(remoteAddress), m_remotePort(remotePort) {}

//         std::string localAddress() const { return m_localAddress; }
//         uint16_t localPort() const { return m_localPort; }
//         std::string remoteAddress() const { return m_remoteAddress; }
//         uint16_t remotePort() const { return m_remotePort; }

//         void close();
//         void send(void *buffer, size_t nbyte);
// };

int main() {
    cout << "makehuman.js mediapipe/notochord daemon" << endl;
    auto orb = make_shared<CORBA::ORB>();
    // auto protocol = new WSProtocol();
    orb->bind("Backend", make_shared<Backend_impl>(orb.get()));
    // after looking at developer:~/test-mico/test.cc: LoginTest::_narrow()
    // CORBA does not require to register a STUB, instead Backend::_narrow(CORBA::Object_ptr) creates it!!!
    // when it's a local object(?) just duplicate the pointer
    // if it's in a repository (?) or remote, create a stub
    // more details here: https://omniorb.sourceforge.io/omnipy42/omniORBpy/omniORBpy003.html#sec%3Anarrowing
    // when an object is received, the IDL generated code can call _narrow to create the stub.

    // orb->registerStub();
    orb->run();
    return 0;
}

// void mediapipeLoop() {
//     IGMOD *test = CreateGMOD();

//     test->set_camera_props(0, 640, 480, 30);
//     test->set_camera(true);
//     test->set_overlay(true);

//     // FOR GRAPHS THAT RETURN IMAGES
//     // The current frame is stored as demo.png in the folder
//     // This requires the graph to have ecoded_output_video ( or whatever stream name you put here ) as an output stream
//     // Normally mediapipe graphs return mediapipe::ImageFrame as output, you can use the ImageFrameToOpenCVMatCalculator
//     // to do convert it to a cv::Mat.
//     //////////////////////////////////
//     // auto obs = test->create_observer("encoded_output_video");
//     // obs->SetPresenceCallback([](class IObserver* observer, bool present){});
//     // obs->SetPacketCallback([](class IObserver* observer){
//     //     string message_type = observer->GetMessageType();
//     //     cout << message_type << endl;
//     //     cv::Mat* test;
//     //     test = static_cast<cv::Mat*>(observer->GetData());
//     //     cv::imwrite("testing.png", *test);
//     // });
//     //////////////////////////////////

//     // FOR GRAPH THAT RETURNS NORMALIZED LANDMARK LIST
//     //////////////////////////////////
//     // auto obs = test->create_observer("face_landmarks");
//     // obs->SetPresenceCallback([](class IObserver* observer, bool present){});
//     // obs->SetPacketCallback([](class IObserver* observer){
//     //     string message_type = observer->GetMessageType();
//     //     cout << message_type << endl;
//     //     auto data = static_cast<mediapipe::NormalizedLandmarkList*>(observer->GetData());
//     //     cout << data->landmark(0).x() << endl;
//     // });
//     //////////////////////////////////

//     // FOR GRAPH THAT RETURNS MULTIPLE NORMALIZED LANDMARK LISTS
//     //////////////////////////////////
//     auto obs = test->create_observer("multi_face_landmarks");
//     obs->SetPacketCallback([](class IObserver *observer) { cout << "packet" << endl; });
//     obs->SetPresenceCallback([](class IObserver *observer, bool present) { cout << "present = " << (present ? "true" : "false") << endl; });
//     obs->SetPacketCallback([](class IObserver *observer) {
//         // string message_type = observer->GetMessageType();
//         // cout << message_type << endl;
//         auto multi_face_landmarks = static_cast<MultiFaceLandmarks *>(observer->GetData());
//         auto lm = (*multi_face_landmarks)[0];
//         // Landmark {float: x,y,z,visibility,presence }
//         // cout << lm.landmark_size() << " landmarks: " << lm.landmark(0).x() << ", " << lm.landmark(0).x() << ", " << lm.landmark(0).z() << endl;
//         wsHandle();
//         if (isFaceRequested()) {
//             float float_array[lm.landmark_size() * 3];
//             for (int i = 0; i < lm.landmark_size(); ++i) {
//                 float_array[i * 3] = lm.landmark(i).x();
//                 float_array[i * 3 + 1] = lm.landmark(i).y();
//                 float_array[i * 3 + 2] = lm.landmark(1).z();
//             }
//             sendFace(float_array, lm.landmark_size() * 3);
//         }
//     });
//     //////////////////////////////////

//     // These are the graphs that have been tested and work

//     // HOLISTIC TRACKING
//     // test->start("mediapipe_graphs/holistic_tracking/holistic_tracking_cpu.pbtxt");
//     // HAND TRACKING
//     // test->start("mediapipe_graphs/hand_tracking/hand_detection_desktop_live.pbtxt");
//     // test->start("mediapipe_graphs/hand_tracking/hand_tracking_desktop_live.pbtxt");
//     // POSE TRACKING
//     // test->start("mediapipe_graphs/pose_tracking/pose_tracking_cpu.pbtxt");
//     // IRIS TRACKING
//     // test->start("mediapipe_graphs/iris_tracking/iris_tracking_cpu.pbtxt");
//     // test->start("mediapipe_graphs/iris_tracking/iris_tracking_gpu.pbtxt");
//     // FACE DETECTION
//     // test->start("mediapipe_graphs/face_detection/face_detection_desktop_live.pbtxt");
//     // test->start("mediapipe_graphs/face_detection/face_detection_full_range_desktop_live.pbtxt");
//     // FACE MESH
//     test->start("mediapipe_graphs/face_mesh/face_mesh_desktop_live.pbtxt");
//     // SELFIE SEGMENTATION
//     // test->start("mediapipe_graphs/selfie_segmentation/selfie_segmentation_cpu.pbtxt");
//     // HAIR SEGMENTATION
//     // test->start("mediapipe_graphs/hair_segmentation/hair_segmentation_desktop_live.pbtxt");
// }
