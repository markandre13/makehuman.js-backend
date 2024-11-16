#include "renderer.hh"

@implementation Renderer : NSObject
- (nonnull instancetype)initWithMetalKitView:(nonnull MTKView *)aView {
    NSAssert(NO, @"initWithMetalKitView: needs to be implemented");
    return self;
}
- (void)invalidate {
    // [_view setNeedsDisplay: YES];
    [_view draw];
    // [self drawInMTKView: _view];
}
- (void)drawInMTKView:(nonnull MTKView *)view {
    NSAssert(NO, @"drawInMTKView: needs to be implemented");
}
@end
