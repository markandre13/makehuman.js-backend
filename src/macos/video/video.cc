#include "../../makehuman_skel.hh"

#include <opencv2/opencv.hpp>
#import <AVFoundation/AVFoundation.h>

#include <print>
#include <vector>

using namespace std;

struct DimensionFPS {
    Float64 fps;
    int32_t width, height;
};

class VideoCamera_impl : public VideoCamera2_skel {
        int openCvIndex;
        std::string _id;
        std::string _name;
        std::string _features;
    public:
        VideoCamera_impl(int openCvIndex, std::string_view &id, std::string_view name, const std::string &features): _id(id), _name(name), _features(features) {}
        CORBA::async<std::string> id();
        CORBA::async<std::string> name();
        CORBA::async<void> name(const std::string_view &);
        CORBA::async<std::string> features();
};

CORBA::async<std::string> VideoCamera_impl::id() {
    co_return _id;
}
CORBA::async<std::string> VideoCamera_impl::name() {
    co_return _name;
}
CORBA::async<void> VideoCamera_impl::name(const std::string_view &name) {
    _name = name;
    co_return;
}
CORBA::async<std::string> VideoCamera_impl::features() {
    co_return _features;
}

/**
 * Create list of available video cameras
 */
std::vector<std::shared_ptr<VideoCamera2>>
getVideoCameras(std::shared_ptr<CORBA::ORB> orb) {
    std::vector<std::shared_ptr<VideoCamera2>> cameras;

    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

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

    unsigned openCVDeviceID = 0;
    for (AVCaptureDevice * d in devices) {
        string_view id(d.uniqueID.UTF8String);
        string_view name(d.localizedName.UTF8String);

        bool first = true;
        DimensionFPS maxDimension;
        DimensionFPS maxFPS;

        // println("id '{}', name '{}'", id, name);
        for (AVCaptureDeviceFormat *format in d.formats) {
            CMVideoDimensions formatDimensions = CMVideoFormatDescriptionGetDimensions(format.formatDescription);
            // println("  {} x {}", formatDimensions.width, formatDimensions.height);

            NSArray<AVFrameRateRange*>* ranges = [format videoSupportedFrameRateRanges];
            for (int k = 0; k < [ranges count]; ++k) {
                AVFrameRateRange* range = [ranges objectAtIndex: k];
                // println("   {} {}", [range minFrameRate], [range maxFrameRate]);
                Float64 fps = [range maxFrameRate];
                if (first) {
                    first = false;
                    maxDimension.fps = maxFPS.fps = [range maxFrameRate];
                    maxDimension.width = maxFPS.width = formatDimensions.width;
                    maxDimension.height = maxFPS.height = formatDimensions.height;
                    continue;
                }

                if (fps > maxFPS.fps) {
                    maxFPS.fps = fps;
                    maxFPS.width = formatDimensions.width;
                    maxFPS.height = formatDimensions.height; 
                } else 
                if (fps == maxFPS.fps) {
                    if (maxFPS.width <= formatDimensions.width && maxFPS.height <= formatDimensions.height) {
                        maxFPS.width = formatDimensions.width;
                        maxFPS.height = formatDimensions.height; 
                    }
                }

                if (maxDimension.width <= formatDimensions.width && maxDimension.height <= formatDimensions.height) {
                    maxDimension.fps = fps;
                    maxDimension.width = formatDimensions.width;
                    maxDimension.height = formatDimensions.height; 
                } else
                if (maxDimension.width == formatDimensions.width && maxDimension.height == formatDimensions.height) {
                    if (fps > maxDimension.fps) {
                        maxDimension.fps = fps;
                    }
                }
            }
        }
        string features;
        if (fabs(maxFPS.fps - maxDimension.fps)<0.01 
            && maxFPS.width == maxDimension.width
            && maxFPS.height == maxDimension.height)
        {
            features = format("{}x{}@{:.2f}", maxFPS.width, maxFPS.height, maxFPS.fps);

        } else {
            features = format("{}x{}@{:.2f} to {}x{}@{:.2f}",
                maxDimension.width, maxDimension.height, maxDimension.fps,
                maxFPS.width, maxFPS.height, maxFPS.fps
            );
        }

        println("{} {}", name, features);
        auto camera = make_shared<VideoCamera_impl>(openCVDeviceID, id, name, features);
        orb->activate_object(camera);
        cameras.push_back(camera);

        ++openCVDeviceID;
    }

    [pool drain];

    return cameras;
}
