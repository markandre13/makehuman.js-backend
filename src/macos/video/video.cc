#include <opencv2/opencv.hpp>

#import <AVFoundation/AVFoundation.h>
// #import <CoreMedia/CMFormatDescription.h>

#include <print>
#include <vector>

using namespace std;

void getVideoInputs() {
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

    //
    // LIST DEVICES
    //

    // create a device list similar to the one used internally by opencv 3/4 on macos.
    // for reference see:
    //   opencv/modules/videoio/src/cap_avfoundation_mac.mm
    //   CvCaptureCAM::startCaptureDevice(int cameraNum)
    NSArray *devices = [
        [AVCaptureDevice devicesWithMediaType: AVMediaTypeVideo]
            arrayByAddingObjectsFromArray: [ AVCaptureDevice devicesWithMediaType: AVMediaTypeMuxed ]
    ];
    devices = [devices
        sortedArrayUsingComparator: ^NSComparisonResult(AVCaptureDevice *d1, AVCaptureDevice *d2) {
            return [d1.uniqueID compare:d2.uniqueID];
        }
    ];

    int deviceId = -1;

    int cameraNum = 0;
    for (AVCaptureDevice * device in devices) {
        string_view id(device.uniqueID.UTF8String);
        string_view name(device.localizedName.UTF8String);
        println("device '{}' '{}' cameraNum={}", id, name, cameraNum);
        // if (name == "Brio 500") {
        // if (name == "MX Brio") {
        if (name == "Logitech BRIO") {
        // if (name == "Logi Capture") {
            println("  GOT IT");
            deviceId = cameraNum;
        }
        ++cameraNum;
    }

    //
    // DEVICE FEATURES (DIMENSIONS, FRAMES PER SECOND)
    //

    AVCaptureDevice *device = [AVCaptureDevice deviceWithUniqueID: [NSString stringWithUTF8String: useId.c_str()]];
    println("USE DEVICE '{}'", device != nullptr);

    for (AVCaptureDeviceFormat *format in device.formats) {
        FourCharCode formatSubType = CMFormatDescriptionGetMediaSubType(format.formatDescription);
        auto formatDescription = stringFromSubType(formatSubType);
        auto isFullRange = isFullRangeFormat(formatSubType);

        CMVideoDimensions formatDimensions = CMVideoFormatDescriptionGetDimensions(format.formatDescription);

        // if (formatDescription == "YUY2 - 422YpCbCr8_yuvs" && 
        //     formatDimensions.width == 3840 &&
        //      formatDimensions.height == 2160
        // ) { 

        bool unknownFrameRate = true;
        Float64 minFrameRate = 0.0, maxFrameRate = 0.0; 
        NSArray<AVFrameRateRange*>* ranges = [format videoSupportedFrameRateRanges];
        for (int k = 0; k < [ranges count]; ++k) {
            AVFrameRateRange* range = [ranges objectAtIndex: k];
            // println("   {} {}", [range minFrameRate], [range maxFrameRate]);
            if (unknownFrameRate) {
                unknownFrameRate = false;
                minFrameRate = [range minFrameRate];
                maxFrameRate = [range maxFrameRate];
            } else {
                minFrameRate = min(minFrameRate, [range minFrameRate]);
                maxFrameRate = max(maxFrameRate, [range maxFrameRate]);
            }
        }
        // println("{} ({:x}), {} x {}, {} to {} fps",
        //     formatDescription,
        //     formatSubType,
        //     formatDimensions.width, formatDimensions.height,
        //     minFrameRate, maxFrameRate
        // );
    }

    //
    // OKAY, NOW THAT WE GOT A NICE CAM WITHOUT OPENCV... WE ALSO HAVE TO GET THE VIDEO WITHOUT OPENCV...
    // LOOK AT THE OPENCV CODE!!!
    //

    for(int deviceID=0; deviceID<devices.count; ++deviceID) {
        cv::VideoCapture cap;
        int apiID = cv::CAP_ANY;

        cap.open(deviceID, apiID);
        if (!cap.isOpened()) {
            // throw runtime_error(format("failed to open video device {}, api {}", deviceID, apiID));
            break;
        }

        // MX Brio: 1920 x 1080 max 60 fps
        // cap.set(cv::CAP_PROP_FRAME_WIDTH, 1920);
        // cap.set(cv::CAP_PROP_FRAME_HEIGHT, 1080);
        // cap.set(cv::CAP_PROP_FPS, 60);

        // Logitech BRIO
        cap.set(cv::CAP_PROP_FRAME_WIDTH, 4096);
        cap.set(cv::CAP_PROP_FRAME_HEIGHT, 2160);
        cap.set(cv::CAP_PROP_FPS, 30);

        auto backendName = cap.getBackendName();
        double w = cap.get(cv::CAP_PROP_FRAME_WIDTH);
        double h = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
        double fps = cap.get(cv::CAP_PROP_FPS);
        println("opened video capture device {} {}: {}x{}, {} fps", deviceID, backendName.c_str(), w, h, fps);
    }

    [pool drain];
    // return 0;
    // }
}

// in the UI:
// device    : <name> (max: XxX, X fps))
// resolution: XxX (max. X fps)
// fps       : X fps
