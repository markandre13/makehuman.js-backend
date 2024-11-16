#include "target.hh"

#include <print>

#include "algorithms.hh"
using namespace std;

void Target::diff(const vector<float>& src, const vector<float>& dst) {
    if (src.size() != dst.size()) {
        throw runtime_error(format("Target.diff(src, dst): src and dst must have the same length but they are {} and {}", src.size(), dst.size()));
    }
    for (size_t v = 0, i = 0; v < src.size(); v += 3, ++i) {
        auto s = simd::make_float3(src[v], src[v + 1], src[v + 2]);
        auto d = simd::make_float3(dst[v], dst[v + 1], dst[v + 2]);
        auto vec = d - s;
        if (!isZero(vec.x) || !isZero(vec.y) || !isZero(vec.z)) {
            index.push_back(i);
            verts.push_back(vec);
        }
    }
}

void Target::apply(vector<shader_types::VertexData>& dst, float scale) {
    for (size_t i = 0; i < index.size(); ++i) {
        dst[index[i]].position += verts[i] * scale;
    }
}
