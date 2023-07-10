#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>

#include "EventHandler.hh"
#include "gmod_api.h"
#include "mediapipe/framework/formats/landmark.pb.h"

// On the Notochord's "Home" is an IP and PORT to which UDP will be send when started
// c++ main.cc && ./a.out

// using namespace std;

using std::string, std::vector, std::cout, std::endl;

bool isFaceRequested();
bool isChordataRequested();
void sendFace(float *float_array, int size);
void sendChordata(void* data, size_t size);

typedef const vector<::mediapipe::NormalizedLandmarkList> MultiFaceLandmarks;

class ChordataRecvHandler : public EventHandler {
    public:
        ChordataRecvHandler(int fd) : fd_(fd) {}
        virtual ~ChordataRecvHandler();
        virtual int on_read_event();
        virtual int on_write_event() { return 0; }
        virtual bool want_read() { return true; }
        virtual bool want_write() { return false; }
        virtual int fd() const { return fd_; }
        virtual const char *name() const { return "HttpHandshakeRecvHandler"; }
        virtual bool finish() { return !accept_key_.empty(); }
        virtual EventHandler *next() { return nullptr; }

    private:
        int fd_;
        std::string headers_;
        std::string accept_key_;
};

void hexdump(unsigned char *buffer, int received) {
    int data = 0;
    while (data < received) {
        for (int x = 0; x < 16; x++) {
            if (data < received)
                printf("%02x ", (int)buffer[data]);
            else
                printf("   ");
            data++;
        }
        data -= 16;
        for (int x = 0; x < 16; x++) {
            if (data < received)
                printf("%c", buffer[data] >= 32 && buffer[data] <= 127 ? buffer[data] : '.');
            else
                printf(" ");
            data++;
        }
        printf("\n");
    }
}

ChordataRecvHandler::~ChordataRecvHandler() {
    std::cout << "ChordataRecvHandler: close" << std::endl;
    shutdown(fd_, SHUT_WR);
    close(fd_);
}

extern std::set<EventHandler *> handlers;

char* chordata_buf = nullptr;
ssize_t chordata_len = 0;

int ChordataRecvHandler::on_read_event() {
    sockaddr_in client;
    static char buf[4096];
    socklen_t client_address_size = sizeof(client);
    ssize_t len = recvfrom(fd_, buf, sizeof(buf), 0, (struct sockaddr *)&client, &client_address_size);
    if (len < 0) {
        perror("recvfrom");
        return 1;
    }
    // hexdump((unsigned char *)buf, len);
    chordata_buf = buf;
    chordata_len = len;
    // printf("chordata rcvd %zd octets\n", len);
    return 0;
}

void chordataInit() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    int yes = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

    sockaddr_in name;
    name.sin_family = AF_INET;
    name.sin_addr.s_addr = htonl(INADDR_ANY);
    name.sin_port = htons(6565);

    if (bind(sock, (sockaddr *)&name, sizeof(sockaddr_in)) < 0) {
        perror("bind");
        close(sock);
        sock = -1;
        exit(1);
    }

    handlers.insert(new ChordataRecvHandler(sock));
}

int main() {
    cout << "makehuman.js mediapipe/notochord daemon is running" << endl;
    wsInit();
    chordataInit();
    printf("running\n");

    while (true) {
        wsHandle(true);
        if (isChordataRequested() && chordata_buf) {
            // printf("chordata send %zd octets\n", chordata_len);
            // hexdump((unsigned char*)chordata_buf, chordata_len);
            sendChordata(chordata_buf, chordata_len);
            chordata_buf = nullptr;
        }
    }

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
    obs->SetPacketCallback([](class IObserver *observer) {
        // string message_type = observer->GetMessageType();
        // cout << message_type << endl;
        auto multi_face_landmarks = static_cast<MultiFaceLandmarks *>(observer->GetData());
        auto lm = (*multi_face_landmarks)[0];
        // Landmark {float: x,y,z,visibility,presence }
        // cout << lm.landmark_size() << " landmarks: " << lm.landmark(0).x() << ", " << lm.landmark(0).x() << ", " << lm.landmark(0).z() << endl;
        wsHandle();
        if (isFaceRequested()) {
            float float_array[lm.landmark_size() * 3];
            for (int i = 0; i < lm.landmark_size(); ++i) {
                float_array[i * 3] = lm.landmark(i).x();
                float_array[i * 3 + 1] = lm.landmark(i).y();
                float_array[i * 3 + 2] = lm.landmark(1).z();
            }
            sendFace(float_array, lm.landmark_size() * 3);
        }
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
