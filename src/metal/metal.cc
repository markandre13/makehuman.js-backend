#import <Cocoa/Cocoa.h>
#import <MetalKit/MetalKit.h>

#include <map>
#include <print>

#include "../mesh/wavefront.hh"
#include "algorithms.hh"
#include "renderapp.hh"

// morph target
class Target {
        std::vector<unsigned> index;
        std::vector<simd::float3> verts;

    public:
        /**
         * calculate morph target from two lists of vertices
         *
         * @param src
         * @param dst
         */
        void diff(const std::vector<float>& src, const std::vector<float>& dst);

        /**
         * apply morph target to vertices
         *
         * @param dst destination
         * @param scale a value between 0 and 1
         */
        void apply(const std::vector<simd::float3>& dst, float scale);
};

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
        void loadBlendShapes();
};

static constexpr size_t kNumInstances = 1;
static constexpr size_t kMaxFramesInFlight = 1;
float _angle = 0.00f;

using namespace std;

namespace shader_types {
struct VertexData {
        simd::float3 position;
        simd::float3 normal;
};

struct InstanceData {
        simd::float4x4 instanceTransform;
        simd::float3x3 instanceNormalTransform;
        simd::float4 instanceColor;
};

struct CameraData {
        simd::float4x4 perspectiveTransform;
        simd::float4x4 worldTransform;
        simd::float3x3 worldNormalTransform;
};
}  // namespace shader_types

namespace math {
constexpr simd::float3 add(const simd::float3& a, const simd::float3& b);
constexpr simd_float4x4 makeIdentity();
simd::float4x4 makePerspective(float fovRadians, float aspect, float znear, float zfar);
simd::float4x4 makeXRotate(float angleRadians);
simd::float4x4 makeYRotate(float angleRadians);
simd::float4x4 makeZRotate(float angleRadians);
simd::float4x4 makeTranslate(const simd::float3& v);
simd::float4x4 makeScale(const simd::float3& v);
simd::float3x3 discardTranslation(const simd::float4x4& m);
}  // namespace math

@interface TriangleRenderer : Renderer {
    id<MTLLibrary> _library;
    id<MTLDepthStencilState> _pDepthStencilState;
    id<MTLBuffer> _pVertexDataBuffer;
    id<MTLBuffer> _pIndexBuffer;
    id<MTLBuffer> _pInstanceDataBuffer;
    id<MTLBuffer> _pCameraDataBuffer;

    size_t indiceCount;
    FaceRenderer *faceRenderer;
}
@end

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
    auto &obj = faceRenderer->neutral;
    println("NEUTRAL {}", obj.vcount.size());
    // WavefrontObj obj("upstream/makehuman.js/base/3dobjs/cube.obj");
    auto nxyz = calculateNormals(obj.vcount, obj.fxyz, obj.xyz);
    vector<shader_types::VertexData> verts(obj.fxyz.size());
    const float s = 150.0;
    for (size_t i = 0; i < obj.xyz.size() / 3; ++i) {
        auto i3 = i * 3;
        verts[i].position.x = obj.xyz[i3] * s;
        verts[i].position.y = obj.xyz[i3 + 1] * s;
        verts[i].position.z = obj.xyz[i3 + 2] * s;
        verts[i].normal = nxyz[i];
    }
    auto indices = triangles(obj.vcount, obj.fxyz);
    println("INDICES {}", indices.size());
    indiceCount = indices.size();

    const size_t vertexDataSize = verts.size() * sizeof(shader_types::VertexData);
    const size_t indexDataSize = indices.size() * sizeof(uint16_t);
    _pVertexDataBuffer = [_device newBufferWithLength:vertexDataSize options:MTLResourceStorageModeManaged];
    _pIndexBuffer = [_device newBufferWithLength:indexDataSize options:MTLResourceStorageModeManaged];
    memcpy([_pVertexDataBuffer contents], verts.data(), vertexDataSize);
    memcpy([_pIndexBuffer contents], indices.data(), indexDataSize);

    [_pVertexDataBuffer didModifyRange:NSMakeRange(0, [_pVertexDataBuffer length])];
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
    using simd::float3;
    using simd::float4;
    using simd::float4x4;

    // println("metal view draw");

    id pool = [NSAutoreleasePool new];
    id<MTLCommandBuffer> commandBuffer = [_commandQueue commandBuffer];

    _angle += 0.01f;
    const float scl = 0.1f;

    shader_types::InstanceData* pInstanceData = reinterpret_cast<shader_types::InstanceData*>([_pInstanceDataBuffer contents]);
    float3 objectPosition = {0.f, 0.f, -5.f};

    // Update instance positions:
    float4x4 rt = math::makeTranslate(objectPosition);
    float4x4 rr = math::makeYRotate(-_angle);
    float4x4 rtInv = math::makeTranslate({-objectPosition.x, -objectPosition.y, -objectPosition.z});
    float4x4 fullObjectRot = rt * rr * rtInv;

    for (size_t i = 0; i < kNumInstances; ++i) {
        float angle = 0.0f;
        float iDivNumInstances = i / (float)kNumInstances;
        float xoff = (iDivNumInstances * 2.0f - 1.0f) + (1.f / kNumInstances);
        float yoff = sin((iDivNumInstances + angle) * 2.0f * M_PI);
        float4x4 scale = math::makeScale((float3){scl, scl, scl});
        float4x4 zrot = math::makeZRotate(_angle);
        float4x4 yrot = math::makeYRotate(_angle);
        float4x4 translate = math::makeTranslate(math::add(objectPosition, {xoff, yoff, 0.f}));
        pInstanceData[i].instanceTransform = fullObjectRot * translate * yrot * zrot * scale;
        pInstanceData[i].instanceNormalTransform = math::discardTranslation(pInstanceData[i].instanceTransform);

        float r = iDivNumInstances;
        float g = 1.0f - r;
        float b = sinf(M_PI * 2.0f * iDivNumInstances);
        pInstanceData[i].instanceColor = (float4){r, g, b, 1.0f};
    }
    [_pInstanceDataBuffer didModifyRange:NSMakeRange(0, [_pInstanceDataBuffer length])];

    // Update camera state:
    // MTL::Buffer* pCameraDataBuffer = _pCameraDataBuffer[ _frame ];
    shader_types::CameraData* pCameraData = reinterpret_cast<shader_types::CameraData*>([_pCameraDataBuffer contents]);
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

/////////////////////////////////////////////

int main() {
    println("Metal...");

    id pool = [NSAutoreleasePool new];
    id app = [NSApplication sharedApplication];

    RenderAppDelegate* delegate = [[RenderAppDelegate alloc] init:[TriangleRenderer alloc]];
    [app setDelegate:delegate];

    [app run];
    [pool release];

    return 0;
}

namespace math {
constexpr simd::float3 add(const simd::float3& a, const simd::float3& b) { return {a.x + b.x, a.y + b.y, a.z + b.z}; }

constexpr simd_float4x4 makeIdentity() {
    using simd::float4;
    return (simd_float4x4){(float4){1.f, 0.f, 0.f, 0.f}, (float4){0.f, 1.f, 0.f, 0.f}, (float4){0.f, 0.f, 1.f, 0.f}, (float4){0.f, 0.f, 0.f, 1.f}};
}

simd::float4x4 makePerspective(float fovRadians, float aspect, float znear, float zfar) {
    using simd::float4;
    float ys = 1.f / tanf(fovRadians * 0.5f);
    float xs = ys / aspect;
    float zs = zfar / (znear - zfar);
    return simd_matrix_from_rows((float4){xs, 0.0f, 0.0f, 0.0f}, (float4){0.0f, ys, 0.0f, 0.0f}, (float4){0.0f, 0.0f, zs, znear * zs}, (float4){0, 0, -1, 0});
}

simd::float4x4 makeXRotate(float angleRadians) {
    using simd::float4;
    const float a = angleRadians;
    return simd_matrix_from_rows((float4){1.0f, 0.0f, 0.0f, 0.0f}, (float4){0.0f, cosf(a), sinf(a), 0.0f}, (float4){0.0f, -sinf(a), cosf(a), 0.0f},
                                 (float4){0.0f, 0.0f, 0.0f, 1.0f});
}

simd::float4x4 makeYRotate(float angleRadians) {
    using simd::float4;
    const float a = angleRadians;
    return simd_matrix_from_rows((float4){cosf(a), 0.0f, sinf(a), 0.0f}, (float4){0.0f, 1.0f, 0.0f, 0.0f}, (float4){-sinf(a), 0.0f, cosf(a), 0.0f},
                                 (float4){0.0f, 0.0f, 0.0f, 1.0f});
}

simd::float4x4 makeZRotate(float angleRadians) {
    using simd::float4;
    const float a = angleRadians;
    return simd_matrix_from_rows((float4){cosf(a), sinf(a), 0.0f, 0.0f}, (float4){-sinf(a), cosf(a), 0.0f, 0.0f}, (float4){0.0f, 0.0f, 1.0f, 0.0f},
                                 (float4){0.0f, 0.0f, 0.0f, 1.0f});
}

simd::float4x4 makeTranslate(const simd::float3& v) {
    using simd::float4;
    const float4 col0 = {1.0f, 0.0f, 0.0f, 0.0f};
    const float4 col1 = {0.0f, 1.0f, 0.0f, 0.0f};
    const float4 col2 = {0.0f, 0.0f, 1.0f, 0.0f};
    const float4 col3 = {v.x, v.y, v.z, 1.0f};
    return simd_matrix(col0, col1, col2, col3);
}

simd::float4x4 makeScale(const simd::float3& v) {
    using simd::float4;
    return simd_matrix((float4){v.x, 0, 0, 0}, (float4){0, v.y, 0, 0}, (float4){0, 0, v.z, 0}, (float4){0, 0, 0, 1.0});
}

simd::float3x3 discardTranslation(const simd::float4x4& m) { return simd_matrix(m.columns[0].xyz, m.columns[1].xyz, m.columns[2].xyz); }

}  // namespace math

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
    println("LOAD BLEND SHAPES");
    for (auto blendshape : blendshapeNames) {
        if (blendshape == "_neutral") {
            continue;
        }
        WavefrontObj obj(format("/Users/mark/public_html/makehuman.js/base/blendshapes/arkit/{}.obj", blendshape));
        auto bs = blendshapes.emplace(blendshape, BlendShape());
        bs.first->second.target.diff(neutral.xyz, obj.xyz);
    }
}

template <class T>
inline bool isZero(T a) {
    return fabsf(a) < std::numeric_limits<T>::epsilon();
}

void Target::diff(const vector<float>& src, const vector<float>& dst) {
    if (src.size() != dst.size()) {
        throw runtime_error(format("Target.diff(src, dst): src and dst must have the same length but they are {} and {}", src.size(), dst.size()));
    }
    for (size_t v = 0, i = 0; v < src.size(); v += 3, ++i) {
        auto s = simd::make_float3(src[v], src[v + 1], src[v + 2]);
        auto d = simd::make_float3(dst[v], dst[v + 1], dst[v + 2]);
        auto vec = d - s;
        if (!isZero(vec.x) || !isZero(vec.y) || !isZero(vec.z)) {
            index.push_back(i);
            verts.push_back(vec);
        }
    }
}

void Target::apply(const vector<simd::float3>& dst, float scale) {}