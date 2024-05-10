#include "kaffeeklatsch.hh"
#include "../src/mesh/wavefront.hh"

using namespace kaffeeklatsch;

using namespace std;

kaffeeklatsch_spec([] {
    describe("class WavefrontOBJ", [] {
        it("load file", [] {
            WavefrontObj obj("mesh/foo.obj");
            expect(obj.xyz).to.equal(vector<float>{
                -1.1, -1.2, 1.3,
                1.0, -1.0, 1.0,
                1.0, 1.0, 1.0,
                -1.0, 1.0, 1.0,

                -1.0, -1.0, -1.0,
                1.0, -1.0, -1.0,
                1.0, 1.0, -1.0,
                -1.0, 1.0, -1.0,
            });
            expect(obj.fxyz).to.equal(vector<unsigned>{
                0, 1, 2, 3,
                4, 5, 6, 7
            });
            expect(obj.uv).to.equal(vector<float>{
                0, 0,
                1, 0,
                1, 1,
                0, 1,
            });
            expect(obj.fuv).to.equal(vector<unsigned>{
                0, 1, 2, 3,
                0, 1, 2, 3,
            });
            expect(obj.vcount).to.equal(vector<unsigned>{
                4, 4,
            });
        });
    });
});
