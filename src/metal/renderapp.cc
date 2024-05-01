#include "renderapp.hh"
#include <print>

// build with help from:
// https://www.cocoawithlove.com/2010/09/minimalist-cocoa-programming.html

using namespace std;

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
    view.clearColor = MTLClearColorMake(0.1, 0.1, 0.1, 1.0);
    view.colorPixelFormat = MTLPixelFormatBGRA8Unorm_sRGB;
    view.depthStencilPixelFormat = MTLPixelFormatDepth16Unorm;
    view.clearDepth = 1.0f;
    view.enableSetNeedsDisplay = YES;

    [_renderer initWithMetalKitView:view];
    view.delegate = _renderer;

    [window setContentView:view];
}

@end
