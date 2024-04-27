#import <Cocoa/Cocoa.h>
#import <MetalKit/MetalKit.h>

#include <print>

using namespace std;

/////////////////////////////////////////////

@interface Renderer : NSObject <MTKViewDelegate> {
    MTKView *_view;
    id<MTLDevice> _device;
    id<MTLCommandQueue> _commandQueue;
    id<MTLRenderPipelineState> _pPSO;
}
- (nonnull instancetype)initWithMetalKitView:(nonnull MTKView *)aView;
@end

@implementation Renderer : NSObject
- (nonnull instancetype)initWithMetalKitView:(nonnull MTKView *)aView {
    NSAssert(NO, @"initWithMetalKitView: needs to be implemented");
    return self;
}
@end

/////////////////////////////////////////////

@interface TriangleRenderer : Renderer {
    id<MTLBuffer> _pVertexPositionsBuffer;
    id<MTLBuffer> _pVertexColorsBuffer;
}
@end

@implementation TriangleRenderer : Renderer
- (nonnull instancetype)initWithMetalKitView:(nonnull MTKView *)aView {
    if (self = [super init]) {
        if (!aView.device) {
            println("ERROR: NO DEVICE IN VIEW");
        }
        _view = aView;
        _device = _view.device;
        _commandQueue = [_device newCommandQueue];
        [self buildShaders];
        [self buildBuffers];
    }
    return self;
}
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

            v2f vertex vertexMain( uint vertexId [[vertex_id]],
                                device const float3* positions [[buffer(0)]],
                                device const float3* colors [[buffer(1)]] )
            {
                v2f o;
                o.position = float4( positions[ vertexId ], 1.0 );
                o.color = half3 ( colors[ vertexId ] );
                return o;
            }

            half4 fragment fragmentMain( v2f in [[stage_in]] )
            {
                return half4( in.color, 1.0 );
            }
        )";

    NSError *error;
    id<MTLLibrary> pLibrary = [_device newLibraryWithSource:[NSString stringWithUTF8String:shaderSrc] options:nil error:&error];
    if (!pLibrary) {
        [NSException raise:@"Failed to compile shaders" format:@"%@", [error localizedDescription]];
    }

    id<MTLFunction> pVertexFn = [pLibrary newFunctionWithName:@"vertexMain"];
    id<MTLFunction> pFragFn = [pLibrary newFunctionWithName:@"fragmentMain"];

    MTLRenderPipelineDescriptor *pDesc = [[MTLRenderPipelineDescriptor alloc] init];
    pDesc.vertexFunction = pVertexFn;
    pDesc.fragmentFunction = pFragFn;
    pDesc.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm_sRGB;

    _pPSO = [_device newRenderPipelineStateWithDescriptor:pDesc error:&error];
    if (!_pPSO) {
        [NSException raise:@"Failed to create pipeline state" format:@"%@", [error localizedDescription]];
    }

    [pVertexFn release];
    [pFragFn release];
    [pDesc release];
    [pLibrary release];
}
- (void)buildBuffers {
    println("build buffers");

    const size_t NumVertices = 3;

    simd::float3 positions[NumVertices] = {{-0.8f, 0.8f, 0.0f}, {0.0f, -0.8f, 0.0f}, {+0.8f, 0.8f, 0.0f}};
    simd::float3 colors[NumVertices] = {{1.0, 0.3f, 0.2f}, {0.8f, 1.0, 0.0f}, {0.8f, 0.0f, 1.0}};

    const size_t positionsDataSize = NumVertices * sizeof(simd::float3);
    const size_t colorDataSize = NumVertices * sizeof(simd::float3);

    id<MTLBuffer> pVertexPositionsBuffer = [_device newBufferWithLength:positionsDataSize options:MTLResourceStorageModeManaged];
    id<MTLBuffer> pVertexColorsBuffer = [_device newBufferWithLength:colorDataSize options:MTLResourceStorageModeManaged];

    _pVertexPositionsBuffer = pVertexPositionsBuffer;
    _pVertexColorsBuffer = pVertexColorsBuffer;

    memcpy([_pVertexPositionsBuffer contents], positions, positionsDataSize );
    memcpy([_pVertexColorsBuffer contents], colors, colorDataSize );

    [_pVertexPositionsBuffer didModifyRange: NSMakeRange( 0, [_pVertexPositionsBuffer length])];
    [_pVertexColorsBuffer didModifyRange: NSMakeRange( 0, [_pVertexColorsBuffer length])];
}
- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size {
    println("metal view size {}x{}", size.width, size.height);
}
- (void)drawInMTKView:(nonnull MTKView *)pView {
    println("metal view draw");
    id pool = [NSAutoreleasePool new];

    id<MTLCommandBuffer> commandBuffer = [_commandQueue commandBuffer];

    MTLRenderPassDescriptor* passDescriptor = [pView currentRenderPassDescriptor];

    id<MTLRenderCommandEncoder> commandEncoder = [commandBuffer renderCommandEncoderWithDescriptor:passDescriptor];
    [commandEncoder setRenderPipelineState:_pPSO];
    [commandEncoder setVertexBuffer:_pVertexPositionsBuffer offset:0 atIndex:0];
    [commandEncoder setVertexBuffer:_pVertexColorsBuffer offset:0 atIndex:1];
    [commandEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:3];
    [commandEncoder endEncoding];

    id<CAMetalDrawable> drawable = [pView currentDrawable];
    [commandBuffer presentDrawable:drawable];
    [commandBuffer commit];

    [pool release];
}

@end

/////////////////////////////////////////////
@interface RenderAppDelegate : NSObject <NSApplicationDelegate> {
    Renderer *_renderer;
}
// - (void)applicationWillFinishLaunching:(NSNotification *)notification;
// - (void)applicationDidFinishLaunching:(NSNotification *)notification;
// - (void)createWindow;
// - (void)createMenu;
// - (void) windowWillMove:(NSNotification *)notification;
// - (void) windowDidMove:(NSNotification *)notification;
@end

@implementation RenderAppDelegate : NSObject
- (id)init: (Renderer*) renderer {
    _renderer = renderer;
    return self;
}
- (void)applicationWillFinishLaunching:(NSNotification *)notification
{
    println("applicationWillFinishLaunching...");
    // normal app with menu and menu item
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    [self createMenu];
}
- (void)applicationDidFinishLaunching:(NSNotification *)notification
{
    println("applicationDidFinishLaunching...");
    [self createWindow];
    [NSApp activateIgnoringOtherApps:YES];
}
- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)Sender {
    return YES;
}

- (void)createMenu {
    println("create menu...");
    id menubar = [[NSMenu new] autorelease];
    id appMenuItem = [[NSMenuItem new] autorelease];
    [menubar addItem:appMenuItem];
    [NSApp setMainMenu:menubar];

    id appMenu = [[NSMenu new] autorelease];
    id appName = [[NSProcessInfo processInfo] processName];
    id quitTitle = [@"Quit " stringByAppendingString:appName];
    id quitMenuItem = [[[NSMenuItem alloc] initWithTitle:quitTitle action:@selector(terminate:) keyEquivalent:@"q"] autorelease];
    [appMenu addItem:quitMenuItem];
    [appMenuItem setSubmenu:appMenu];
}

- (void)createWindow {
    //
    // NEXTSTEP WINDOW
    //
    println("create window...");
    NSRect frame = NSMakeRect(0, 0, 640, 480);
    id window =
        [[NSWindow alloc] initWithContentRect:frame
                                    styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable
                                      backing:NSBackingStoreBuffered
                                        defer:NO];
    [window cascadeTopLeftFromPoint:NSMakePoint(20, 20)];

    id appName = [[NSProcessInfo processInfo] processName];
    [window setTitle:appName];

    [window makeKeyAndOrderFront:NSApp];

    //
    // METAL VIEW
    //
    id<MTLDevice> device;
    device = MTLCreateSystemDefaultDevice();
    if (!device) {
        println("no metal default device");
    } else {
        println("metal default device: {}", [[device name] cStringUsingEncoding:NSISOLatin1StringEncoding]);
    }

    MTKView *view = [[MTKView alloc] initWithFrame:frame device:device];
    view.clearColor = MTLClearColorMake(0.0, 0.5, 1.0, 1.0);
    view.colorPixelFormat = MTLPixelFormatBGRA8Unorm_sRGB;
    view.enableSetNeedsDisplay = YES;

    [_renderer initWithMetalKitView:view];
    view.delegate = _renderer;

    [window setContentView:view];
}

@end

int main() {
    println("Metal...");

    id pool = [NSAutoreleasePool new];
    id app = [NSApplication sharedApplication];

    Renderer *renderer = [TriangleRenderer alloc];

    RenderAppDelegate *delegate = [[RenderAppDelegate alloc] init: renderer];
    [app setDelegate:delegate];
    // [app activateIgnoringOtherApps:YES];

    [app run];
    [pool release];

    return 0;
}
