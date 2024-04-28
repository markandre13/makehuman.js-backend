#import <Cocoa/Cocoa.h>
#import <MetalKit/MetalKit.h>

#include <print>

#include "renderapp.hh"

using namespace std;

@interface TriangleRenderer : Renderer {
    id<MTLLibrary> _library;
    id<MTLBuffer> _pArgBuffer;  // buffer of buffers to reuse buffers uploaded to the GPU between renders
    id<MTLBuffer> _pVertexPositionsBuffer;
    id<MTLBuffer> _pVertexColorsBuffer;
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

            // argument buffer
            struct VertexData {
                device float3* positions [[id(0)]];
                device float3* colors [[id(1)]];
            };

            // vertex shader out
            struct v2f {
                float4 position [[position]];
                half3 color;
            };

            // we get two buffers from the app and a vertex number
            v2f vertex vertexMain(
                  device const VertexData* vertexData [[buffer(0)]],
                  uint vertexId [[vertex_id]]
            ) {
                v2f o;
                // o.position = float4( positions[ vertexId ], 1.0 );
                // o.color = half3 ( colors[ vertexId ] );
                o.position = float4( vertexData->positions[ vertexId ], 1.0 );
                o.color = half3(vertexData->colors[ vertexId ]);
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

    const size_t nVertices = 3;

    simd::float3 positions[nVertices] = {{-0.8f, 0.8f, 0.0f}, {0.0f, -0.8f, 0.0f}, {+0.8f, 0.8f, 0.0f}};
    simd::float3 colors[nVertices] = {{1.0, 0.3f, 0.2f}, {0.8f, 1.0, 0.0f}, {0.8f, 0.0f, 1.0}};

    const size_t nbytesPositions = nVertices * sizeof(simd::float3);
    const size_t nbytesColors = nVertices * sizeof(simd::float3);

    _pVertexPositionsBuffer = [_device newBufferWithLength:nbytesPositions options:MTLResourceStorageModeManaged];
    _pVertexColorsBuffer = [_device newBufferWithLength:nbytesColors options:MTLResourceStorageModeManaged];

    memcpy([_pVertexPositionsBuffer contents], positions, nbytesPositions);
    memcpy([_pVertexColorsBuffer contents], colors, nbytesColors);

    [_pVertexPositionsBuffer didModifyRange:NSMakeRange(0, [_pVertexPositionsBuffer length])];
    [_pVertexColorsBuffer didModifyRange:NSMakeRange(0, [_pVertexColorsBuffer length])];

    // build argument buffer
    id<MTLFunction> functionGettingTheArgumentBuffer = [_library newFunctionWithName:@"vertexMain"];
    NSUInteger indexOfArgumentBufferInFunctionsArgumentList = 0;
    id<MTLArgumentEncoder> argumentEncoder = [functionGettingTheArgumentBuffer newArgumentEncoderWithBufferIndex:indexOfArgumentBufferInFunctionsArgumentList];
    _pArgBuffer = [_device newBufferWithLength:[argumentEncoder encodedLength] options:MTLResourceStorageModeManaged];
    [argumentEncoder setArgumentBuffer:_pArgBuffer startOffset: 0 arrayElement: 0];
    [argumentEncoder setBuffer: _pVertexPositionsBuffer offset:0 atIndex: 0];
    [argumentEncoder setBuffer: _pVertexColorsBuffer offset:0 atIndex: 1];
    [_pArgBuffer didModifyRange:NSMakeRange(0, [_pArgBuffer length])];
    
    [functionGettingTheArgumentBuffer release];
    [argumentEncoder release];
}
- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size {
    println("metal view size {}x{}", size.width, size.height);
}
- (void)drawInMTKView:(nonnull MTKView *)pView {
    println("metal view draw");
    id pool = [NSAutoreleasePool new];

    id<MTLCommandBuffer> commandBuffer = [_commandQueue commandBuffer];

    MTLRenderPassDescriptor *passDescriptor = [pView currentRenderPassDescriptor];

    id<MTLRenderCommandEncoder> commandEncoder = [commandBuffer renderCommandEncoderWithDescriptor:passDescriptor];
    [commandEncoder setRenderPipelineState:_pPSO];
    // [commandEncoder setVertexBuffer:_pVertexPositionsBuffer offset:0 atIndex:0];
    // [commandEncoder setVertexBuffer:_pVertexColorsBuffer offset:0 atIndex:1];
    [commandEncoder setVertexBuffer:_pArgBuffer offset:0 atIndex:0];
    [commandEncoder useResource:_pVertexPositionsBuffer usage: MTLResourceUsageRead];
    [commandEncoder useResource:_pVertexColorsBuffer usage: MTLResourceUsageRead];

    [commandEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:3];
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
