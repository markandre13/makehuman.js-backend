#pragma once

#include "renderer.hh"

@interface RenderAppDelegate : NSObject <NSApplicationDelegate> {
    Renderer *_renderer;
}
- (id)init: (Renderer*) renderer;
// - (void)applicationWillFinishLaunching:(NSNotification *)notification;
// - (void)applicationDidFinishLaunching:(NSNotification *)notification;
// - (void)createWindow;
// - (void)createMenu;
// - (void) windowWillMove:(NSNotification *)notification;
// - (void) windowDidMove:(NSNotification *)notification;
@end