#pragma once

#include "renderer.hh"

#ifdef HAVE_MEDIAPIPE
#include <cc_lib/mediapipe.hh>
#endif

#include <optional>
#include <span>

class MetalFacerenderer;
class FaceRenderer;

@interface TriangleRenderer : Renderer {
    id<MTLLibrary> _library;
    id<MTLDepthStencilState> _pDepthStencilState;
    id<MTLBuffer> _pVertexDataBuffer;
    id<MTLBuffer> _pIndexBuffer;
    id<MTLBuffer> _pInstanceDataBuffer;
    id<MTLBuffer> _pCameraDataBuffer;

    size_t indiceCount;
@public
    FaceRenderer* faceRenderer;
}
@end

class MetalFacerenderer {
    public:
        TriangleRenderer* delegate;
        void faceLandmarks(std::optional<mediapipe::cc_lib::vision::face_landmarker::FaceLandmarkerResult> result, int64_t timestamp_ms);
};

MetalFacerenderer* metal();
