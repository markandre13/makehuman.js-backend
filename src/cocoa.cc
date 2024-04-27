#import <Cocoa/Cocoa.h>
#import <MetalKit/MetalKit.h>

#include <print>

using namespace std;

@interface toadView : NSView <NSTextInputClient> {
@public
    void *twindow;
}
@end

@implementation toadView : NSView
- (BOOL)isFlipped {
    return TRUE;
}
- (BOOL)isOpaque {
    return TRUE;
}
@end

/////////////////////////////////////////////

@interface Renderer : NSObject <MTKViewDelegate> {
    MTKView *view;
    id<MTLDevice> device;
    id<MTLCommandQueue> commandQueue;
}
@end

@implementation Renderer : NSObject
- (nonnull instancetype)initWithMetalKitView:(nonnull MTKView *)aView {
    if (self = [super init]) {
        // Initialize self
        view = aView;
        commandQueue = [view.device newCommandQueue];
    }
    return self;
}
- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size {
    println("metal view size {}x{}", size.width, size.height);
}
- (void)drawInMTKView:(nonnull MTKView *)view {
    println("metal view draw");
    id pool = [NSAutoreleasePool new];

    id<MTLCommandBuffer> cmd = [commandQueue commandBuffer];
    // MTL::RenderPassDescriptor* pRpd = pView->currentRenderPassDescriptor();
    // MTL::RenderCommandEncoder* pEnc = pCmd->renderCommandEncoder( pRpd );
    // pEnc->endEncoding();
    // pCmd->presentDrawable( pView->currentDrawable() );
    [cmd presentDrawable:[self->view currentDrawable]];
    // pCmd->commit();
    [cmd commit];

    [pool release];
}

@end

/////////////////////////////////////////////
@interface ToadDelegate : NSObject <NSApplicationDelegate> {
}
- (void)applicationWillFinishLaunching:(NSNotification *)notification;
- (void)applicationDidFinishLaunching:(NSNotification *)notification;
- (void)createWindow;
- (void)createMenu;
// - (void) windowWillMove:(NSNotification *)notification;
// - (void) windowDidMove:(NSNotification *)notification;
@end

@implementation ToadDelegate : NSObject

- (void)applicationWillFinishLaunching:(NSNotification *)notification;
{
    println("applicationWillFinishLaunching...");
    // normal app with menu and menu item
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    [self createMenu];
}
- (void)applicationDidFinishLaunching:(NSNotification *)notification;
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
    println("create window...");
    // toadView *view = [[toadView alloc] initWithFrame:NSMakeRect(0, 0, 320, 200)];
    id window =
        [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, 640, 480)
                                    styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable
                                      backing:NSBackingStoreBuffered
                                        defer:NO];
    [window cascadeTopLeftFromPoint:NSMakePoint(20, 20)];
    // [window setBackgroundColor:[NSColor blueColor]];

    id appName = [[NSProcessInfo processInfo] processName];
    [window setTitle:appName];

    [window makeKeyAndOrderFront:NSApp];

    //
    // METAL
    //

    id<MTLDevice> device;
    device = MTLCreateSystemDefaultDevice();
    if (!device) {
        println("no metal default device\n");
    } else {
        println("metal default device: {}\n", [[device name] cStringUsingEncoding:NSISOLatin1StringEncoding]);
    }

    MTKView *view = [MTKView new];
    view.device = device;
    view.clearColor = MTLClearColorMake(0.0, 0.5, 1.0, 1.0);
    view.enableSetNeedsDisplay = YES;

    Renderer *renderer = [[Renderer alloc] initWithMetalKitView: view];
    // Renderer *renderer = [Renderer new];
    view.delegate = renderer;

    [window setContentView:view];
}

@end

int main() {
    println("Metal...");

    id pool = [NSAutoreleasePool new];
    id app = [NSApplication sharedApplication];

    ToadDelegate *delegate = [ToadDelegate new];
    [app setDelegate:delegate];
    // [app activateIgnoringOtherApps:YES];

    [app run];
    [pool release];

    return 0;
}
