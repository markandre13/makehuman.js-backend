#import <Cocoa/Cocoa.h>

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

@interface ToadDelegate : NSObject <NSApplicationDelegate> {
}
- (void)createWindow;
- (void)createMenu;
// - (void) applicationWillFinishLaunching:(NSNotification *)notification;
- (void)applicationDidFinishLaunching:(NSNotification *)notification;
// - (void) windowWillMove:(NSNotification *)notification;
// - (void) windowDidMove:(NSNotification *)notification;
@end

@implementation ToadDelegate : NSObject

- (void)createMenu {
    println("create menu...");
    // normal app with menu and menu item
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

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
    id window = [[NSWindow alloc]
        initWithContentRect:NSMakeRect(0, 0, 640, 480)
                  styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable
                    backing:NSBackingStoreBuffered
                      defer:NO];
    [window cascadeTopLeftFromPoint:NSMakePoint(20,20)];
    [window setBackgroundColor:[NSColor blueColor]];

    id appName = [[NSProcessInfo processInfo] processName];
    [window setTitle: appName];

    [window makeKeyAndOrderFront:NSApp];
}

- (void)applicationWillFinishLaunching:(NSNotification *)notification;
{
    println("applicationWillFinishLaunching...");
    [self createMenu];
    [self createWindow];
}
- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)Sender {
    return YES;
}

@end

int main() {
    println("Metal...");

    id pool = [NSAutoreleasePool new];
    id app = [NSApplication sharedApplication];

    ToadDelegate *delegate = [ToadDelegate new];
    [app setDelegate:delegate];
    [app activateIgnoringOtherApps:YES];

    [app run];
    [pool release];

    return 0;
}
