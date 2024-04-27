#include "renderer.hh"

@implementation Renderer : NSObject
- (nonnull instancetype)initWithMetalKitView:(nonnull MTKView *)aView {
    NSAssert(NO, @"initWithMetalKitView: needs to be implemented");
    return self;
}
- (void)drawInMTKView:(nonnull MTKView *)view {
    NSAssert(NO, @"drawInMTKView: needs to be implemented");
}
@end
