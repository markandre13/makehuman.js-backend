#pragma once

#import <Cocoa/Cocoa.h>
#import <MetalKit/MetalKit.h>

@interface Renderer : NSObject <MTKViewDelegate> {
    MTKView *_view;
    id<MTLDevice> _device;
    id<MTLCommandQueue> _commandQueue;
    id<MTLRenderPipelineState> _pPSO;
}
- (nonnull instancetype)initWithMetalKitView:(nonnull MTKView *)View;
- (void)drawInMTKView:(nonnull MTKView *)view;
@end
