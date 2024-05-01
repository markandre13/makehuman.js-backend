#import <Cocoa/Cocoa.h>
#import <MetalKit/MetalKit.h>

#include <print>

#include "renderapp.hh"

static constexpr size_t kNumInstances = 32;
static constexpr size_t kMaxFramesInFlight = 1;

using namespace std;

namespace shader_types {
    struct InstanceData
    {
        simd::float4x4 instanceTransform;
        simd::float4 instanceColor;
    };
}

@interface TriangleRenderer : Renderer {
    id<MTLLibrary> _library;
    id<MTLBuffer> _pArgBuffer;  // buffer of buffers to reuse buffers uploaded to the GPU between renders
    id<MTLBuffer> _pVertexDataBuffer;
    id<MTLBuffer> _pIndexBuffer;
    id<MTLBuffer> _pInstanceDataBuffer;
}
@end

@implementation TriangleRenderer : Renderer
- (nonnull instancetype)initWithMetalKitView:(nonnull MTKView *)aView {
    if (self = [super init]) {
        _view = aView;
        _device = _view.device;
        _commandQueue = [_device newCommandQueue];
        [self buildShaders];
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

        struct v2f
        {
            float4 position [[position]];
            half3 color;
        };

        struct VertexData
        {
            float3 position;
        };

        struct InstanceData
        {
            float4x4 instanceTransform;
            float4 instanceColor;
        };

        v2f vertex vertexMain( device const VertexData* vertexData [[buffer(0)]],
                               device const InstanceData* instanceData [[buffer(1)]],
                               uint vertexId [[vertex_id]],
                               uint instanceId [[instance_id]] )
        {
            v2f o;
            float4 pos = float4( vertexData[ vertexId ].position, 1.0 );
            o.position = instanceData[ instanceId ].instanceTransform * pos;
            o.color = half3( instanceData[ instanceId ].instanceColor.rgb );
            return o;
        }

        half4 fragment fragmentMain( v2f in [[stage_in]] )
        {
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

    _pPSO = [_device newRenderPipelineStateWithDescriptor:pDesc error:&error];
    if (!_pPSO) {
        [NSException raise:@"Failed to create pipeline state" format:@"%@", [error localizedDescription]];
    }

    // [pVertexFn release];
    // [pFragFn release];
    [pDesc release];
}
- (void)buildBuffers {
    println("build buffers");

    using simd::float3;
    const float s = 0.5f;
    // prettier-ignore
    float3 verts[] = {
        { -s, -s, +s },
        { +s, -s, +s },
        { +s, +s, +s },
        { -s, +s, +s }
    };
    uint16_t indices[] = {
        0, 1, 2,
        2, 3, 0,
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
}

- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size {
    println("metal view size {}x{}", size.width, size.height);
}
- (void)drawInMTKView:(nonnull MTKView *)pView {
    using simd::float4;
    using simd::float4x4;

    println("metal view draw");

    id pool = [NSAutoreleasePool new];
    id<MTLCommandBuffer> commandBuffer = [_commandQueue commandBuffer];

    const float _angle = 0.01f;
    const float scl = 0.1f;

    shader_types::InstanceData* pInstanceData = reinterpret_cast< shader_types::InstanceData *>([_pInstanceDataBuffer contents] );
    for ( size_t i = 0; i < kNumInstances; ++i )
    {
        float iDivNumInstances = i / (float)kNumInstances;
        float xoff = (iDivNumInstances * 2.0f - 1.0f) + (1.f/kNumInstances);
        float yoff = sin( ( iDivNumInstances + _angle ) * 2.0f * M_PI);
        pInstanceData[ i ].instanceTransform = (float4x4){ (float4){ scl * sinf(_angle), scl * cosf(_angle), 0.f, 0.f },
                                                           (float4){ scl * cosf(_angle), scl * -sinf(_angle), 0.f, 0.f },
                                                           (float4){ 0.f, 0.f, scl, 0.f },
                                                           (float4){ xoff, yoff, 0.f, 1.f } };

        float r = iDivNumInstances;
        float g = 1.0f - r;
        float b = sinf( M_PI * 2.0f * iDivNumInstances );
        pInstanceData[ i ].instanceColor = (float4){ r, g, b, 1.0f };
    }
    [_pInstanceDataBuffer didModifyRange:NSMakeRange(0, [_pInstanceDataBuffer length])];
    MTLRenderPassDescriptor *passDescriptor = [pView currentRenderPassDescriptor];
    id<MTLRenderCommandEncoder> commandEncoder = [commandBuffer renderCommandEncoderWithDescriptor:passDescriptor];
    [commandEncoder setRenderPipelineState:_pPSO];
    [commandEncoder setVertexBuffer:_pVertexDataBuffer offset:0 atIndex:0];
    [commandEncoder setVertexBuffer:_pInstanceDataBuffer offset:0 atIndex:1];
    [commandEncoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                               indexCount:6
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
