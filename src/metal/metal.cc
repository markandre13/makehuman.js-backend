#import <Cocoa/Cocoa.h>
#import <MetalKit/MetalKit.h>

#include <print>

#include "renderapp.hh"

static constexpr size_t kNumInstances = 32;
static constexpr size_t kMaxFramesInFlight = 1;
float _angle = 0.00f;

using namespace std;

namespace shader_types {
    struct InstanceData {
        simd::float4x4 instanceTransform;
        simd::float4 instanceColor;
    };

    struct CameraData {
        simd::float4x4 perspectiveTransform;
        simd::float4x4 worldTransform;
    };
}

namespace math
{
    constexpr simd::float3 add( const simd::float3& a, const simd::float3& b );
    constexpr simd_float4x4 makeIdentity();
    simd::float4x4 makePerspective(float fovRadians, float aspect, float znear, float zfar);
    simd::float4x4 makeXRotate( float angleRadians );
    simd::float4x4 makeYRotate( float angleRadians );
    simd::float4x4 makeZRotate( float angleRadians );
    simd::float4x4 makeTranslate( const simd::float3& v );
    simd::float4x4 makeScale( const simd::float3& v );
}

@interface TriangleRenderer : Renderer {
    id<MTLLibrary> _library;
    id<MTLDepthStencilState> _pDepthStencilState;
    id<MTLBuffer> _pVertexDataBuffer;
    id<MTLBuffer> _pIndexBuffer;
    id<MTLBuffer> _pInstanceDataBuffer;
    id<MTLBuffer> _pCameraDataBuffer;
}
@end

@implementation TriangleRenderer : Renderer
- (nonnull instancetype)initWithMetalKitView:(nonnull MTKView *)aView {
    if (self = [super init]) {
        _view = aView;
        _device = _view.device;
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

    const char *shaderSrc = R"(
        #include <metal_stdlib>
        using namespace metal;

        struct v2f {
            float4 position [[position]];
            half3 color;
        };

        struct VertexData {
            float3 position;
        };

        struct InstanceData {
            float4x4 instanceTransform;
            float4 instanceColor;
        };

        struct CameraData {
            float4x4 perspectiveTransform;
            float4x4 worldTransform;
        };

        v2f vertex vertexMain( device const VertexData* vertexData [[buffer(0)]],
                               device const InstanceData* instanceData [[buffer(1)]],
                               device const CameraData& cameraData [[buffer(2)]],
                               uint vertexId [[vertex_id]],
                               uint instanceId [[instance_id]] )
        {
            v2f o;
            float4 pos = float4( vertexData[ vertexId ].position, 1.0 );
            pos = instanceData[ instanceId ].instanceTransform * pos;
            pos = cameraData.perspectiveTransform * cameraData.worldTransform * pos;
            o.position = pos;
            o.color = half3( instanceData[ instanceId ].instanceColor.rgb );
            return o;
        }

        half4 fragment fragmentMain( v2f in [[stage_in]] ) {
            return half4( in.color, 1.0 );
        }
    )";

    // compile shaders
    NSError *error;
    _library = [_device newLibraryWithSource:[NSString stringWithUTF8String:shaderSrc] options:nil error:&error];
    if (!_library) {
        [NSException raise:@"Failed to compile shaders" format:@"%@", [error localizedDescription]];
    }

    MTLRenderPipelineDescriptor *pDesc = [MTLRenderPipelineDescriptor new];
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
    MTLDepthStencilDescriptor *pDsDesc = [MTLDepthStencilDescriptor new];
    [pDsDesc setDepthCompareFunction: MTLCompareFunctionLess];
    [pDsDesc setDepthWriteEnabled: TRUE];
    _pDepthStencilState = [_device newDepthStencilStateWithDescriptor: pDsDesc];
    [pDsDesc release];
}
- (void)buildBuffers {
    println("build buffers");

    using simd::float3;
    const float s = 2.0f;
    float3 verts[] = {
        { -s, -s, +s },
        { +s, -s, +s },
        { +s, +s, +s },
        { -s, +s, +s },

        { -s, -s, -s },
        { -s, +s, -s },
        { +s, +s, -s },
        { +s, -s, -s }
    };

    uint16_t indices[] = {
        0, 1, 2, /* front */
        2, 3, 0,

        1, 7, 6, /* right */
        6, 2, 1,

        7, 4, 5, /* back */
        5, 6, 7,

        4, 0, 3, /* left */
        3, 5, 4,

        3, 2, 6, /* top */
        6, 5, 3,

        4, 7, 1, /* bottom */
        1, 0, 4
    };

    const size_t vertexDataSize = sizeof( verts );
    const size_t indexDataSize = sizeof( indices );
    _pVertexDataBuffer = [_device newBufferWithLength:vertexDataSize options:MTLResourceStorageModeManaged];
    _pIndexBuffer = [_device newBufferWithLength:indexDataSize options:MTLResourceStorageModeManaged];
    memcpy( [_pVertexDataBuffer contents], verts, vertexDataSize );
    memcpy( [_pIndexBuffer contents], indices, indexDataSize );

    [_pVertexDataBuffer didModifyRange:NSMakeRange(0, [_pVertexDataBuffer length])];
    [_pIndexBuffer didModifyRange:NSMakeRange(0, [_pIndexBuffer length])];

    const size_t instanceDataSize = kNumInstances * sizeof( shader_types::InstanceData );
    _pInstanceDataBuffer = [_device newBufferWithLength:instanceDataSize options:MTLResourceStorageModeManaged];

    const size_t cameraDataSize = sizeof( shader_types::CameraData );
    _pCameraDataBuffer = [_device newBufferWithLength:cameraDataSize options:MTLResourceStorageModeManaged];
}

- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size {
    println("metal view size {}x{}", size.width, size.height);
}
- (void)drawInMTKView:(nonnull MTKView *)pView {
    using simd::float3;
    using simd::float4;
    using simd::float4x4;

    println("metal view draw");

    id pool = [NSAutoreleasePool new];
    id<MTLCommandBuffer> commandBuffer = [_commandQueue commandBuffer];

    _angle += 0.01f;
    const float scl = 0.1f;

    shader_types::InstanceData* pInstanceData = reinterpret_cast< shader_types::InstanceData *>([_pInstanceDataBuffer contents] );
    float3 objectPosition = { 0.f, 0.f, -5.f };

    // Update instance positions:
    float4x4 rt = math::makeTranslate( objectPosition );
    float4x4 rr = math::makeYRotate( -_angle );
    float4x4 rtInv = math::makeTranslate( { -objectPosition.x, -objectPosition.y, -objectPosition.z } );
    float4x4 fullObjectRot = rt * rr * rtInv;

    for ( size_t i = 0; i < kNumInstances; ++i )
    {
        float angle = 0.0f;
        float iDivNumInstances = i / (float)kNumInstances;
        float xoff = (iDivNumInstances * 2.0f - 1.0f) + (1.f/kNumInstances);
        float yoff = sin( ( iDivNumInstances + angle ) * 2.0f * M_PI);
        float4x4 scale = math::makeScale( (float3){ scl, scl, scl } );
        float4x4 zrot = math::makeZRotate( angle );
        float4x4 yrot = math::makeYRotate( angle );
        float4x4 translate = math::makeTranslate( math::add( objectPosition, { xoff, yoff, 0.f } ) );
        pInstanceData[i].instanceTransform = fullObjectRot * translate * yrot * zrot * scale;

        float r = iDivNumInstances;
        float g = 1.0f - r;
        float b = sinf( M_PI * 2.0f * iDivNumInstances );
        pInstanceData[ i ].instanceColor = (float4){ r, g, b, 1.0f };
    }
    [_pInstanceDataBuffer didModifyRange:NSMakeRange(0, [_pInstanceDataBuffer length])];

    // Update camera state:
    // MTL::Buffer* pCameraDataBuffer = _pCameraDataBuffer[ _frame ];
    shader_types::CameraData* pCameraData = reinterpret_cast< shader_types::CameraData *>([_pCameraDataBuffer contents]);
    // pCameraData->perspectiveTransform = math::makePerspective( 45.f * M_PI / 180.f, 1.f, 0.03f, 500.0f ) ;
    pCameraData->perspectiveTransform = math::makePerspective( 45.f * M_PI / 180.f, 1.f, 0.03f, 500.0f ) ;
    // pCameraData->worldTransform = math::makeIdentity();
    pCameraData->worldTransform = math::makeIdentity();
    // pCameraDataBuffer->didModifyRange( NS::Range::Make( 0, sizeof( shader_types::CameraData ) ) ); 
    [_pCameraDataBuffer didModifyRange:NSMakeRange(0, [_pCameraDataBuffer length])];

    MTLRenderPassDescriptor *passDescriptor = [pView currentRenderPassDescriptor];
    id<MTLRenderCommandEncoder> commandEncoder = [commandBuffer renderCommandEncoderWithDescriptor:passDescriptor];
    [commandEncoder setRenderPipelineState:_pPSO];
    [commandEncoder setDepthStencilState: _pDepthStencilState];
    [commandEncoder setVertexBuffer:_pVertexDataBuffer offset:0 atIndex:0];
    [commandEncoder setVertexBuffer:_pInstanceDataBuffer offset:0 atIndex:1];
    [commandEncoder setVertexBuffer:_pCameraDataBuffer offset:0 atIndex:2];
    [commandEncoder setCullMode: MTLCullModeBack];
    [commandEncoder setFrontFacingWinding: MTLWindingCounterClockwise];
    [commandEncoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                                indexCount:6 * 6
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

    RenderAppDelegate *delegate = [[RenderAppDelegate alloc] init:[TriangleRenderer alloc]];
    [app setDelegate:delegate];

    [app run];
    [pool release];

    return 0;
}

namespace math
{
    constexpr simd::float3 add( const simd::float3& a, const simd::float3& b )
    {
        return { a.x + b.x, a.y + b.y, a.z + b.z };
    }

    constexpr simd_float4x4 makeIdentity()
    {
        using simd::float4;
        return (simd_float4x4){ (float4){ 1.f, 0.f, 0.f, 0.f },
                                (float4){ 0.f, 1.f, 0.f, 0.f },
                                (float4){ 0.f, 0.f, 1.f, 0.f },
                                (float4){ 0.f, 0.f, 0.f, 1.f } };
    }

    simd::float4x4 makePerspective( float fovRadians, float aspect, float znear, float zfar )
    {
        using simd::float4;
        float ys = 1.f / tanf(fovRadians * 0.5f);
        float xs = ys / aspect;
        float zs = zfar / ( znear - zfar );
        return simd_matrix_from_rows((float4){ xs, 0.0f, 0.0f, 0.0f },
                                     (float4){ 0.0f, ys, 0.0f, 0.0f },
                                     (float4){ 0.0f, 0.0f, zs, znear * zs },
                                     (float4){ 0, 0, -1, 0 });
    }

    simd::float4x4 makeXRotate( float angleRadians )
    {
        using simd::float4;
        const float a = angleRadians;
        return simd_matrix_from_rows((float4){ 1.0f, 0.0f, 0.0f, 0.0f },
                                     (float4){ 0.0f, cosf( a ), sinf( a ), 0.0f },
                                     (float4){ 0.0f, -sinf( a ), cosf( a ), 0.0f },
                                     (float4){ 0.0f, 0.0f, 0.0f, 1.0f });
    }

    simd::float4x4 makeYRotate( float angleRadians )
    {
        using simd::float4;
        const float a = angleRadians;
        return simd_matrix_from_rows((float4){ cosf( a ), 0.0f, sinf( a ), 0.0f },
                                     (float4){ 0.0f, 1.0f, 0.0f, 0.0f },
                                     (float4){ -sinf( a ), 0.0f, cosf( a ), 0.0f },
                                     (float4){ 0.0f, 0.0f, 0.0f, 1.0f });
    }

    simd::float4x4 makeZRotate( float angleRadians )
    {
        using simd::float4;
        const float a = angleRadians;
        return simd_matrix_from_rows((float4){ cosf( a ), sinf( a ), 0.0f, 0.0f },
                                     (float4){ -sinf( a ), cosf( a ), 0.0f, 0.0f },
                                     (float4){ 0.0f, 0.0f, 1.0f, 0.0f },
                                     (float4){ 0.0f, 0.0f, 0.0f, 1.0f });
    }

    simd::float4x4 makeTranslate( const simd::float3& v )
    {
        using simd::float4;
        const float4 col0 = { 1.0f, 0.0f, 0.0f, 0.0f };
        const float4 col1 = { 0.0f, 1.0f, 0.0f, 0.0f };
        const float4 col2 = { 0.0f, 0.0f, 1.0f, 0.0f };
        const float4 col3 = { v.x, v.y, v.z, 1.0f };
        return simd_matrix( col0, col1, col2, col3 );
    }

    simd::float4x4 makeScale( const simd::float3& v )
    {
        using simd::float4;
        return simd_matrix((float4){ v.x, 0, 0, 0 },
                           (float4){ 0, v.y, 0, 0 },
                           (float4){ 0, 0, v.z, 0 },
                           (float4){ 0, 0, 0, 1.0 });
    }

}