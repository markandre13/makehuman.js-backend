#pragma once

#include "shadertypes.hh"
#include <vector>

// morph target
class Target {
        // fixme: make this one vector
        std::vector<unsigned> index;
        std::vector<simd::float3> verts;

    public:
        /**
         * calculate morph target from two lists of vertices
         *
         * @param src
         * @param dst
         */
        void diff(const std::vector<float>& src, const std::vector<float>& dst);

        /**
         * apply morph target to vertices
         *
         * @param dst destination
         * @param scale a value between 0 and 1
         */
        void apply(std::vector<shader_types::VertexData>& dst, float scale);
};
