#import <Cocoa/Cocoa.h>

#include <print>

using namespace std;

@interface toadView : NSView <NSTextInputClient>
{
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

@interface ToadDelegate : NSObject <NSApplicationDelegate> {
}
- (void)createWindow;
// - (void) createMenu;
// - (void) applicationWillFinishLaunching:(NSNotification *)notification;
- (void)applicationDidFinishLaunching:(NSNotification *)notification;
// - (void) windowWillMove:(NSNotification *)notification;
// - (void) windowDidMove:(NSNotification *)notification;
@end

@implementation ToadDelegate : NSObject

- (void)createWindow {
    println("create window...");
    // toadView *view = [[toadView alloc] initWithFrame:NSMakeRect(0, 0, 320, 200)];
    NSRect frame = NSMakeRect(0, 0, 640, 480);
    NSWindow* window  = [[[NSWindow alloc] initWithContentRect:frame
                        styleMask:NSWindowStyleMaskTitled
                                | NSWindowStyleMaskMiniaturizable
                                | NSWindowStyleMaskClosable
                                | NSWindowStyleMaskResizable
                        backing:NSBackingStoreBuffered
                        defer:NO] autorelease];
    [window setBackgroundColor:[NSColor blueColor]];
    [window makeKeyAndOrderFront:NSApp];
    [window setTitle: [NSString stringWithUTF8String: "makehuman.js metal"]];
}

- (void)applicationWillFinishLaunching:(NSNotification *)notification;
{
    println("applicationWillFinishLaunching...");
    [self createWindow];
}

@end

int main() {
    println("Metal...");

    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    NSApplication *app = [NSApplication sharedApplication];

    ToadDelegate *delegate = [ToadDelegate new];
    [NSApp setDelegate:delegate];
    [app run];

    [pool release];

    return 0;
}
