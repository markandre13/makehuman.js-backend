#import <AVFoundation/AVFoundation.h>
// #import <CoreMedia/CMFormatDescription.h>

#include <print>

using namespace std;

void getVideoInputs() {
    NSAutoreleasePool* localpool = [[NSAutoreleasePool alloc] init];

    // get video devices
    auto captureDeviceDiscoverySession = [
        AVCaptureDeviceDiscoverySession
            discoverySessionWithDeviceTypes: @[
            AVCaptureDeviceTypeExternalUnknown,
            AVCaptureDeviceTypeBuiltInWideAngleCamera,
        ]
        mediaType:AVMediaTypeVideo
        position:AVCaptureDevicePositionUnspecified
    ];
    NSArray* devices = [captureDeviceDiscoverySession devices];
    auto deviceCount = [devices count];
    println("found {} video capture devices", [devices count]);

    // print video devices
    for (int i = 0; i < deviceCount; ++i) {
        AVCaptureDevice* device = device = [devices objectAtIndex: i];
        println("{}: uniqueID: {}, localizedName: {}, manufacturer: {}, modelID: {}", i,
            [[device uniqueID] UTF8String],
            [[device localizedName] UTF8String],
            [[device manufacturer] UTF8String],
            [[device modelID] UTF8String]
        );
        if([device hasMediaType:AVMediaTypeMuxed] || [device hasMediaType:AVMediaTypeVideo]) {
            println("  ok");
        }

        NSArray<AVCaptureDeviceFormat*>* formats = [device formats];
        bool unknownFrameRate = true;
        Float64 minFrameRate, maxFrameRate;
        for (int j = 0; j < [formats count]; ++j) {
            // println("    {}", j);
            AVCaptureDeviceFormat* format = [formats objectAtIndex: i];

            CMFormatDescriptionRef fd = [format formatDescription];
            CMVideoDimensions vd = CMVideoFormatDescriptionGetDimensions(fd);
            // println("    size {} x {} px", vd.width, vd.height);

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
                    maxFrameRate = min(maxFrameRate, [range maxFrameRate]);
                }
            }
        }
        if (unknownFrameRate) {
            println("        unknown fps");
        } else {
            println("        {} to {} fps", minFrameRate, maxFrameRate);
        }
    }

    // print additional device info
    auto deviceIndex = 0;

    NSError* error;
    AVCaptureDeviceInput* mCaptureDeviceInput = [[AVCaptureDeviceInput alloc] initWithDevice:[devices objectAtIndex:deviceIndex] error:&error];
    NSArray<AVCaptureInputPort*>* ports = mCaptureDeviceInput.ports;
    println("found {} ports", [ports count]);
    for (int i = 0; i < [ports count]; ++i) {
        AVCaptureInputPort* port = [ports objectAtIndex:i];
        CMFormatDescriptionRef format = [port formatDescription];
        // CMFormatDescriptionRef format = [[ports objectAtIndex:0] formatDescription];
        CGSize s1 = CMVideoFormatDescriptionGetPresentationDimensions(format, YES, YES);
        println("    size {} x {} px", s1.width, s1.height);
    }

    // AVCaptureSession* session = [[AVCaptureSession alloc] init];
    // session.sessionPreset = AVCaptureSessionPreset960x540;
    // [session addInput:mCaptureDeviceInput];

    // // Output
    // AVCaptureVideoDataOutput* output = [[AVCaptureVideoDataOutput alloc] init];
    // [session addOutput:output];
    // output.videoSettings = @{(NSString*)kCVPixelBufferPixelFormatTypeKey : @(kCVPixelFormatType_32BGRA)};

    // // Preview Layer
    // AVCaptureVideoPreviewLayer* previewLayer = [[AVCaptureVideoPreviewLayer alloc] initWithSession:session];
    // previewLayer.frame = viewForCamera.bounds;
    // previewLayer.videoGravity = AVLayerVideoGravityResizeAspectFill;
    // [viewForCamera.layer addSublayer:previewLayer];

    // [session startRunning];

    // if ([devices count] == 0) {
    // std::cout << "AV Foundation didn't find any attached Video Input Devices!" << std::endl;

    [localpool drain];
    // return 0;
    // }
}
