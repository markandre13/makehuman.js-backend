#import <AVFoundation/AVFoundation.h>
// #import <CoreMedia/CMFormatDescription.h>

#include <print>
#include <vector>

using namespace std;

bool isFullRangeFormat(FourCharCode pixelFormat) {
    switch (pixelFormat) {
        case kCVPixelFormatType_420YpCbCr8PlanarFullRange:
        case kCVPixelFormatType_420YpCbCr8BiPlanarFullRange:
        case kCVPixelFormatType_420YpCbCr10BiPlanarFullRange:
        case kCVPixelFormatType_422YpCbCr8FullRange:
            return true;
        default:
            return false;
    }
}

// /Library/Developer/CommandLineTools/SDKs/MacOSX10.14.sdk/System/Library/Frameworks/CoreVideo.framework/Versions/A/Headers/CVPixelBuffer.h
string_view stringFromSubType(FourCharCode subtype) {
    switch (subtype) {
        case kCVPixelFormatType_422YpCbCr8:
            return "UYVY - 422YpCbCr8";
        case kCVPixelFormatType_422YpCbCr8_yuvs:
            return "YUY2 - 422YpCbCr8_yuvs";
        case kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange:
        case kCVPixelFormatType_420YpCbCr8BiPlanarFullRange:
            return "NV12 - 420YpCbCr8BiPlanar";
        case kCVPixelFormatType_420YpCbCr10BiPlanarFullRange:
        case kCVPixelFormatType_420YpCbCr10BiPlanarVideoRange:
            return "P010 - 420YpCbCr10BiPlanar";
        case kCVPixelFormatType_32ARGB:
            return "ARGB - 32ARGB";
        case kCVPixelFormatType_32BGRA:
            return "BGRA - 32BGRA";
        case kCVPixelFormatType_32RGBA:
            return "RGBA - 32RGBA";
        case kCVPixelFormatType_24RGB:
            return "RGB - 24RGB";

        case kCMVideoCodecType_Animation:
            return "Apple Animation";
        case kCMVideoCodecType_Cinepak:
            return "Cinepak";
        case kCMVideoCodecType_JPEG:
            return "JPEG";
        case kCMVideoCodecType_JPEG_OpenDML:
            return "MJPEG - JPEG OpenDML";
        case kCMVideoCodecType_SorensonVideo:
            return "Sorenson Video";
        case kCMVideoCodecType_SorensonVideo3:
            return "Sorenson Video 3";
        case kCMVideoCodecType_H263:
            return "H.263";
        case kCMVideoCodecType_H264:
            return "H.264";
        case kCMVideoCodecType_MPEG4Video:
            return "MPEG-4";
        case kCMVideoCodecType_MPEG2Video:
            return "MPEG-2";
        case kCMVideoCodecType_MPEG1Video:
            return "MPEG-1";

        case kCMVideoCodecType_DVCNTSC:
            return "DV NTSC";
        case kCMVideoCodecType_DVCPAL:
            return "DV PAL";
        case kCMVideoCodecType_DVCProPAL:
            return "Panasonic DVCPro Pal";
        case kCMVideoCodecType_DVCPro50NTSC:
            return "Panasonic DVCPro-50 NTSC";
        case kCMVideoCodecType_DVCPro50PAL:
            return "Panasonic DVCPro-50 PAL";
        case kCMVideoCodecType_DVCPROHD720p60:
            return "Panasonic DVCPro-HD 720p60";
        case kCMVideoCodecType_DVCPROHD720p50:
            return "Panasonic DVCPro-HD 720p50";
        case kCMVideoCodecType_DVCPROHD1080i60:
            return "Panasonic DVCPro-HD 1080i60";
        case kCMVideoCodecType_DVCPROHD1080i50:
            return "Panasonic DVCPro-HD 1080i50";
        case kCMVideoCodecType_DVCPROHD1080p30:
            return "Panasonic DVCPro-HD 1080p30";
        case kCMVideoCodecType_DVCPROHD1080p25:
            return "Panasonic DVCPro-HD 1080p25";

        case kCMVideoCodecType_AppleProRes4444:
            return "Apple ProRes 4444";
        case kCMVideoCodecType_AppleProRes422HQ:
            return "Apple ProRes 422 HQ";
        case kCMVideoCodecType_AppleProRes422:
            return "Apple ProRes 422";
        case kCMVideoCodecType_AppleProRes422LT:
            return "Apple ProRes 422 LT";
        case kCMVideoCodecType_AppleProRes422Proxy:
            return "Apple ProRes 422 Proxy";

        default:
            return "Unknown";
    }
}

void getVideoInputs() {
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

    //
    // LIST DEVICES
    //

    auto deviceTypes = @[
        AVCaptureDeviceTypeBuiltInWideAngleCamera,
        AVCaptureDeviceTypeExternalUnknown,
        // AVCaptureDeviceTypeDeskViewCamera
    ];

    string useId;
    for(auto &mediaType: vector{AVMediaTypeVideo, AVMediaTypeMuxed}) {
        auto discoverySession = [AVCaptureDeviceDiscoverySession 
            discoverySessionWithDeviceTypes: deviceTypes
            mediaType: mediaType
            position: AVCaptureDevicePositionUnspecified
        ];
        
        for (AVCaptureDevice *device in [discoverySession devices]) {
            string_view id(device.uniqueID.UTF8String);
            string_view name(device.localizedName.UTF8String);
            println("device '{}' '{}' {}", id, name, name.length());
            // if (name == "Brio 500") {
            if (name == "MX Brio") {
            // if (name == "Logitech BRIO") {
            // if (name == "Logi Capture") {
                println("  GOT IT");
                useId = id;
            }
        }
    }
    println("USE DEVICE WITH ID '{}'", useId);

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
            println("   {} {}", [range minFrameRate], [range maxFrameRate]);
            if (unknownFrameRate) {
                unknownFrameRate = false;
                minFrameRate = [range minFrameRate];
                maxFrameRate = [range maxFrameRate];
            } else {
                minFrameRate = min(minFrameRate, [range minFrameRate]);
                maxFrameRate = max(maxFrameRate, [range maxFrameRate]);
            }
        }
        println("{} ({:x}), {} x {}, {} to {} fps",
            formatDescription,
            formatSubType,
            formatDimensions.width, formatDimensions.height,
            minFrameRate, maxFrameRate);
    }

    //
    // OKAY, NOW THAT WE GOT A NICE CAM WITHOUT OPENCV... WE ALSO HAVE TO GET THE VIDEO WITHOUT OPENCV...
    //

    AVCaptureSession *session = [[AVCaptureSession alloc] init];

    NSError *nserror;
    AVCaptureDeviceInput *videoInput = [AVCaptureDeviceInput deviceInputWithDevice:device error:&nserror];
    if (videoInput == nullptr) {
        println("ERROR: no video input");
        exit(1);
    }

    if ([session canAddInput: videoInput]) {
        [session addInput: videoInput];
    } else {
        println("ERROR: Could not add the video device to the session");
        exit(1);
    }

    // AVCaptureConnection *conn;
    // AVCaptureMovieFileOutput *movieFileOutput;

    [pool drain];
    // return 0;
    // }
}

// in the UI:
// device    : <name> (max: XxX, X fps))
// resolution: XxX (max. X fps)
// fps       : X fps
