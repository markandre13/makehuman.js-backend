#include <simd/simd.h>

#include "../src/mesh/wavefront.hh"
#include "kaffeeklatsch.hh"

using namespace kaffeeklatsch;

using namespace std;



class RenderMesh {

    public:
        RenderMesh(const WavefrontObj &obj) {

        }
};

// constexpr simd::float3 add( const simd::float3& a, const simd::float3& b );

kaffeeklatsch_spec([] {
    fdescribe("class RenderMesh", [] {
        it("do some math", [] {
        // add
        // muladd
            auto v0 = simd::make_float3(1, 2, 3);
            auto v1 = simd::make_float3(4, 5, 6);
            auto r0 = v0 + v1;
            println("{} {} {}", (float)r0.x, (float)r0.y, (float)r0.z);

        //     auto l = simd::length(v0);
        //     auto d = simd::dot(v0, v1);
        //     auto c = simd::cross(v0, v1);
        //     auto n = simd::normalize(v0);
        });
        it("calculate normals", [] {
            //
        });
    });
});
