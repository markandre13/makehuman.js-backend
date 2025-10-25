#include "metal.hh"

#import <Cocoa/Cocoa.h>
#import <MetalKit/MetalKit.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <map>
#include <print>

#include "../../livelink/livelinkframe.hh"
#include "../../mesh/wavefront.hh"
#include "algorithms.hh"
#include "renderapp.hh"
#include "target.hh"

/**
 * render blendshapes natively with apple metal to have something to compare
 * makehuman.js' latency with
 */
class FaceRenderer {
    public:
        FaceRenderer();

        // private:
        struct BlendShape {
                Target target;
                float weight;
        };
        WavefrontObj neutral;
        std::map<std::string, BlendShape> blendshapes;
        float facial_transformation_matrix[16];
        void loadBlendShapes();
};

static constexpr size_t kNumInstances = 1;
static constexpr size_t kMaxFramesInFlight = 1;
float _angle = 0.00f;

using namespace std;

@implementation TriangleRenderer : Renderer
- (nonnull instancetype)initWithMetalKitView:(nonnull MTKView*)aView {
    if (self = [super init]) {
        _view = aView;
        _device = _view.device;
        faceRenderer = new FaceRenderer();
        _commandQueue = [_device newCommandQueue];
        [self buildShaders];
        [self buildDepthStencilStates];
        [self buildBuffers];
    }
    return self;
}

// halfX : float16[X]
// floatX: float32[X]

- (void)buildShaders {
    println("build shaders");

    const char* shaderSrc = R"(
        #include <metal_stdlib>
        using namespace metal;

        struct v2f
        {
            float4 position [[position]];
            float3 normal;
            half3 color;
        };

        struct VertexData
        {
            float3 position;
            float3 normal;
        };

        struct InstanceData
        {
            float4x4 instanceTransform;
            float3x3 instanceNormalTransform;
            float4 instanceColor;
        };

        struct CameraData
        {
            float4x4 perspectiveTransform;
            float4x4 worldTransform;
            float3x3 worldNormalTransform;
        };

        v2f vertex vertexMain( device const VertexData* vertexData [[buffer(0)]],
                               device const InstanceData* instanceData [[buffer(1)]],
                               device const CameraData& cameraData [[buffer(2)]],
                               uint vertexId [[vertex_id]],
                               uint instanceId [[instance_id]] )
        {
            v2f o;

            const device VertexData& vd = vertexData[ vertexId ];
            float4 pos = float4( vd.position, 1.0 );
            pos = instanceData[ instanceId ].instanceTransform * pos;
            pos = cameraData.perspectiveTransform * cameraData.worldTransform * pos;
            o.position = pos;

            float3 normal = instanceData[ instanceId ].instanceNormalTransform * vd.normal;
            normal = cameraData.worldNormalTransform * normal;
            o.normal = normal;

            o.color = half3( instanceData[ instanceId ].instanceColor.rgb );
            return o;
        }

        half4 fragment fragmentMain( v2f in [[stage_in]] )
        {
            // assume light coming from (front-top-right)
            float3 l = normalize(float3( 1.0, 1.0, 0.8 ));
            float3 n = normalize( in.normal );

            float ndotl = saturate( dot( n, l ) );
            return half4( in.color * 0.1 + in.color * ndotl, 1.0 );
        }
    )";

    // compile shaders
    NSError* error;
    _library = [_device newLibraryWithSource:[NSString stringWithUTF8String:shaderSrc] options:nil error:&error];
    if (!_library) {
        [NSException raise:@"Failed to compile shaders" format:@"%@", [error localizedDescription]];
    }

    MTLRenderPipelineDescriptor* pDesc = [MTLRenderPipelineDescriptor new];
    pDesc.vertexFunction = [_library newFunctionWithName:@"vertexMain"];
    pDesc.fragmentFunction = [_library newFunctionWithName:@"fragmentMain"];
    pDesc.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm_sRGB;
    pDesc.depthAttachmentPixelFormat = MTLPixelFormatDepth16Unorm;

    _pPSO = [_device newRenderPipelineStateWithDescriptor:pDesc error:&error];
    if (!_pPSO) {
        [NSException raise:@"Failed to create pipeline state" format:@"%@", [error localizedDescription]];
    }

    // [pVertexFn release];
    // [pFragFn release];
    [pDesc release];
}
- (void)buildDepthStencilStates {
    MTLDepthStencilDescriptor* pDsDesc = [MTLDepthStencilDescriptor new];
    [pDsDesc setDepthCompareFunction:MTLCompareFunctionLess];
    [pDsDesc setDepthWriteEnabled:TRUE];
    _pDepthStencilState = [_device newDepthStencilStateWithDescriptor:pDsDesc];
    [pDsDesc release];
}
- (void)buildBuffers {
    println("build buffers");

    using simd::float3;

    // WavefrontObj obj("upstream/makehuman.js/base/3dobjs/mediapipe_canonical_face_model.obj");
    auto& obj = faceRenderer->neutral;

    const size_t vertexDataSize = obj.xyz.size() * sizeof(shader_types::VertexData);
    _pVertexDataBuffer = [_device newBufferWithLength:vertexDataSize options:MTLResourceStorageModeManaged];

    auto indices = triangles(obj.vcount, obj.fxyz);
    indiceCount = indices.size();

    const size_t indexDataSize = indices.size() * sizeof(uint16_t);
    _pIndexBuffer = [_device newBufferWithLength:indexDataSize options:MTLResourceStorageModeManaged];
    memcpy([_pIndexBuffer contents], indices.data(), indexDataSize);
    [_pIndexBuffer didModifyRange:NSMakeRange(0, [_pIndexBuffer length])];

    const size_t instanceDataSize = kNumInstances * sizeof(shader_types::InstanceData);
    _pInstanceDataBuffer = [_device newBufferWithLength:instanceDataSize options:MTLResourceStorageModeManaged];

    const size_t cameraDataSize = sizeof(shader_types::CameraData);
    _pCameraDataBuffer = [_device newBufferWithLength:cameraDataSize options:MTLResourceStorageModeManaged];
}

- (void)mtkView:(nonnull MTKView*)view drawableSizeWillChange:(CGSize)size {
    // println("metal view size {}x{}", size.width, size.height);
}
- (void)drawInMTKView:(nonnull MTKView*)pView {
    // println("drawInMTKView");

    using simd::float3;
    using simd::float4;
    using simd::float4x4;

    // println("metal view draw");

    id pool = [NSAutoreleasePool new];
    id<MTLCommandBuffer> commandBuffer = [_commandQueue commandBuffer];

    // _angle += 0.01f;
    const float scl = 0.1f;

    auto& obj = faceRenderer->neutral;
    // auto& bs = faceRenderer->blendshapes["jawOpen"];

    vector<shader_types::VertexData> verts(obj.xyz.size());
    for (size_t i = 0; i < obj.xyz.size() / 3; ++i) {
        auto i3 = i * 3;
        verts[i].position.x = obj.xyz[i3];
        verts[i].position.y = obj.xyz[i3 + 1];
        verts[i].position.z = obj.xyz[i3 + 2];
    }

    for (auto& bs : faceRenderer->blendshapes) {
        if (!isZero(bs.second.weight)) {
            bs.second.target.apply(verts, bs.second.weight);
        }
    }

    const float s = 150.0;
    auto nxyz = calculateNormals(obj.vcount, obj.fxyz, obj.xyz);
    for (size_t i = 0; i < obj.xyz.size() / 3; ++i) {
        verts[i].position *= s;
        verts[i].normal = nxyz[i];
    }

    const size_t vertexDataSize = verts.size() * sizeof(shader_types::VertexData);
    // _pVertexDataBuffer = [_device newBufferWithLength:vertexDataSize options:MTLResourceStorageModeManaged];
    memcpy([_pVertexDataBuffer contents], verts.data(), vertexDataSize);
    [_pVertexDataBuffer didModifyRange:NSMakeRange(0, [_pVertexDataBuffer length])];

    shader_types::InstanceData* pInstanceData = reinterpret_cast<shader_types::InstanceData*>([_pInstanceDataBuffer contents]);

    auto& d = faceRenderer->facial_transformation_matrix;
    pInstanceData[0].instanceTransform.columns[0] = (float4){d[0], d[1], d[2], d[3]};
    pInstanceData[0].instanceTransform.columns[1] = (float4){d[4], d[5], d[6], d[7]};
    pInstanceData[0].instanceTransform.columns[2] = (float4){d[8], d[9], d[10], d[11]};
    pInstanceData[0].instanceTransform.columns[3] = (float4){d[12], d[13], d[14] - 10.0f, d[15]};

    pInstanceData[0].instanceNormalTransform = math::discardTranslation(pInstanceData[0].instanceTransform);
    pInstanceData[0].instanceColor = (float4){1.0f, 0.8f, 0.7f, 1.0f};
    [_pInstanceDataBuffer didModifyRange:NSMakeRange(0, [_pInstanceDataBuffer length])];

    // Update camera state:
    // MTL::Buffer* pCameraDataBuffer = _pCameraDataBuffer[ _frame ];
    shader_types::CameraData* pCameraData = reinterpret_cast<shader_types::CameraData*>([_pCameraDataBuffer contents]);
    // TODO: get actual view size to calculate aspect
    pCameraData->perspectiveTransform = math::makePerspective(45.f * M_PI / 180.f, 1.f, 0.03f, 500.0f);
    pCameraData->worldTransform = math::makeIdentity();
    pCameraData->worldNormalTransform = math::discardTranslation(pCameraData->worldTransform);
    [_pCameraDataBuffer didModifyRange:NSMakeRange(0, [_pCameraDataBuffer length])];

    MTLRenderPassDescriptor* passDescriptor = [pView currentRenderPassDescriptor];
    id<MTLRenderCommandEncoder> commandEncoder = [commandBuffer renderCommandEncoderWithDescriptor:passDescriptor];
    [commandEncoder setRenderPipelineState:_pPSO];
    [commandEncoder setDepthStencilState:_pDepthStencilState];
    [commandEncoder setVertexBuffer:_pVertexDataBuffer offset:0 atIndex:0];
    [commandEncoder setVertexBuffer:_pInstanceDataBuffer offset:0 atIndex:1];
    [commandEncoder setVertexBuffer:_pCameraDataBuffer offset:0 atIndex:2];
    // [commandEncoder setCullMode: MTLCullModeBack];
    [commandEncoder setCullMode:MTLCullModeNone];
    [commandEncoder setFrontFacingWinding:MTLWindingCounterClockwise];
    [commandEncoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                               indexCount:indiceCount
                                indexType:MTLIndexTypeUInt16
                              indexBuffer:_pIndexBuffer
                        indexBufferOffset:0
                            instanceCount:kNumInstances];

    [commandEncoder endEncoding];

    id<CAMetalDrawable> drawable = [pView currentDrawable];
    [commandBuffer presentDrawable:drawable];
    [commandBuffer commit];

    [pool release];
}

@end

MetalFacerenderer* metal() {
    println("create metal renderer...");

    // getVideoInputs();

    id app = [NSApplication sharedApplication];
    TriangleRenderer* r = [TriangleRenderer alloc];
    RenderAppDelegate* delegate = [[RenderAppDelegate alloc] init:r];
    [app setDelegate:delegate];
    [app finishLaunching];

    MetalFacerenderer* renderer = new MetalFacerenderer();
    renderer->delegate = r;
    return renderer;
}

#ifdef HAVE_MEDIAPIPE
void MetalFacerenderer::faceLandmarks(std::optional<mediapipe::cc_lib::vision::face_landmarker::FaceLandmarkerResult> result, int64_t timestamp_ms) {
    if (!result.has_value()) {
        return;
    }
    if (!delegate || !delegate->faceRenderer) {
        return;
    }
    if (!result->face_blendshapes.has_value()) {
        return;
    }

    // copy blendshape weights
    auto& bs = result->face_blendshapes->at(0).categories;
    for (auto& cat : bs) {
        if (!cat.category_name.has_value()) {
            println("no category name");
            continue;
        }
        auto x = delegate->faceRenderer->blendshapes.find(*cat.category_name);
        if (x == delegate->faceRenderer->blendshapes.end()) {
            continue;
        }
        x->second.weight = cat.score;
    }

    // copy transformation matrix
    if (result->facial_transformation_matrixes.has_value()) {
        auto& d = result->facial_transformation_matrixes->at(0).data;
        // println("have facial_transformation_matrix");
        // println("{:.2f} {:.2f} {:.2f} {:.2f}", d[0], d[1], d[2], d[3]);
        // println("{:.2f} {:.2f} {:.2f} {:.2f}", d[4], d[5], d[6], d[7]);
        // println("{:.2f} {:.2f} {:.2f} {:.2f}", d[8], d[9], d[10], d[11]);
        // println("{:.2f} {:.2f} {:.2f} {:.2f}", d[12], d[13], d[14], d[15]);
        memcpy(delegate->faceRenderer->facial_transformation_matrix, d, 16 * sizeof(float));
    }

    [delegate invalidate];
}
#endif

void MetalFacerenderer::faceLandmarks(const LiveLinkFrame& frame) {
    for (int i = 0; i < 61; ++i) {
        auto x = delegate->faceRenderer->blendshapes.find(frame.blendshapeNames[i]);
        if (x == delegate->faceRenderer->blendshapes.end()) {
            continue;
        }
        x->second.weight = frame.weights[i];
    }

    static float transform[16]{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0.62, -45, 1};

    memcpy(delegate->faceRenderer->facial_transformation_matrix, transform, 16 * sizeof(float));

    [delegate invalidate];
}

int mainX() {
    println("Metal...");

    id pool = [NSAutoreleasePool new];
    id app = [NSApplication sharedApplication];

    RenderAppDelegate* delegate = [[RenderAppDelegate alloc] init:[TriangleRenderer alloc]];
    [app setDelegate:delegate];

#if 0
    // https://www.cocoawithlove.com/2009/01/demystifying-nsapplication-by.html
    [app finishLaunching];
    while (true) {
        [pool release];
        pool = [NSAutoreleasePool new];

        NSEvent* event = [app nextEventMatchingMask:NSAnyEventMask untilDate:[NSDate distantFuture] inMode:NSDefaultRunLoopMode dequeue:YES];

        [app sendEvent:event];
        [app updateWindows];
    }
#else
    [app run];
#endif
    [pool release];

    return 0;
}

vector<string_view> blendshapeNames{
    "_neutral",             // 0
    "browDownLeft",         // 1
    "browDownRight",        // 2
    "browInnerUp",          // 3
    "browOuterUpLeft",      // 4
    "browOuterUpRight",     // 5
    "cheekPuff",            // 6
    "cheekSquintLeft",      // 7
    "cheekSquintRight",     // 8
    "eyeBlinkLeft",         // 9
    "eyeBlinkRight",        // 10
    "eyeLookDownLeft",      // 11
    "eyeLookDownRight",     // 12
    "eyeLookInLeft",        // 13
    "eyeLookInRight",       // 14
    "eyeLookOutLeft",       // 15
    "eyeLookOutRight",      // 16
    "eyeLookUpLeft",        // 17
    "eyeLookUpRight",       // 18
    "eyeSquintLeft",        // 19
    "eyeSquintRight",       // 20
    "eyeWideLeft",          // 21
    "eyeWideRight",         // 22
    "jawForward",           // 23
    "jawLeft",              // 24
    "jawOpen",              // 25
    "jawRight",             // 26
    "mouthClose",           // 27
    "mouthDimpleLeft",      // 28
    "mouthDimpleRight",     // 29
    "mouthFrownLeft",       // 30
    "mouthFrownRight",      // 31
    "mouthFunnel",          // 32
    "mouthLeft",            // 33
    "mouthLowerDownLeft",   // 34
    "mouthLowerDownRight",  // 35
    "mouthPressLeft",       // 36
    "mouthPressRight",      // 37
    "mouthPucker",          // 38
    "mouthRight",           // 39
    "mouthRollLower",       // 40
    "mouthRollUpper",       // 41
    "mouthShrugLower",      // 42
    "mouthShrugUpper",      // 43
    "mouthSmileLeft",       // 44
    "mouthSmileRight",      // 45
    "mouthStretchLeft",     // 46
    "mouthStretchRight",    // 47
    "mouthUpperUpLeft",     // 48
    "mouthUpperUpRight",    // 49
    "noseSneerLeft",        // 50
    "noseSneerRight"        // 51
};

FaceRenderer::FaceRenderer() : neutral("/Users/mark/public_html/makehuman.js/base/blendshapes/arkit/Neutral.obj") { loadBlendShapes(); }

void FaceRenderer::loadBlendShapes() {
    println("load blend shapes...");
    for (auto& blendshape : blendshapeNames) {
        if (blendshape == "_neutral") {
            continue;
        }
        // println("{}", blendshape);
        WavefrontObj obj(format("/Users/mark/public_html/makehuman.js/base/blendshapes/arkit/{}.obj", blendshape));
        auto bs = blendshapes.emplace(blendshape, BlendShape());
        bs.first->second.target.diff(neutral.xyz, obj.xyz);
    }
}
