#include <simd/simd.h>

#include "../src/mesh/wavefront.hh"
#include "kaffeeklatsch.hh"

using namespace kaffeeklatsch;

using namespace std;

// class RenderMesh {

//     public:
//         RenderMesh(const WavefrontObj &obj) {

//         }
// };

// constexpr simd::float3 add( const simd::float3& a, const simd::float3& b );

struct NormalizedLandmark {
  float x;
  float y;
  float z;
  std::optional<float> visibility = std::nullopt;
  std::optional<float> presence = std::nullopt;
  std::optional<std::string> name = std::nullopt;
};

struct NormalizedLandmarks {
  std::vector<NormalizedLandmark> landmarks;
};

struct Category {
  int index;
  float score;
  std::optional<std::string> category_name = std::nullopt;
  std::optional<std::string> display_name = std::nullopt;
};

struct Classifications {
  std::vector<Category> categories;
  int head_index;
  std::optional<std::string> head_name = std::nullopt;
};

struct FaceLandmarkerResult {
  std::vector<NormalizedLandmarks> face_landmarks;
  std::optional<std::vector<Classifications>> face_blendshapes;
//   std::optional<std::vector<Matrix>> facial_transformation_matrixes;
};

kaffeeklatsch_spec([] {
    describe("class RenderMesh", [] {
        xit("do some simd & math experiments", [] {
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
        xit("calculate normals", [] {
            //
        });
        xit("some c++ experiments...", []{
            FaceLandmarkerResult in {
                .face_landmarks {
                    {
                        .landmarks {
                            {.x = 1.0, .y = 1.1, .z = 1.2 },
                            {.x = 2.0, .y = 2.1, .z = 2.2 },
                            {.x = 3.0, .y = 3.1, .z = 3.2 },    
                        }
                    }
                },
                // .face_blendshapes {
                //     {{
                //         .categories { {
                //             .index = 0,
                //             .score = 0.6,
                //             .category_name { "jawOpen" }
                //         }, {
                //             .index = 1,
                //             .score = 0.4,
                //             .category_name { "mouthClosed" }
                //         } },
                //         .head_index = 0,
                //         .head_name { "first" }
                //     }}
                // }
            };
            in.face_blendshapes = {{}};
            println("has_value = {}", in.face_blendshapes.has_value());

            // in.face_landmarks.emplace_back(NormalizedLandmarks { .landmarks = {
            //     {.x = 1.0, .y = 1.1, .z = 1.2 },
            //     {.x = 2.0, .y = 2.1, .z = 2.2 },
            //     {.x = 3.0, .y = 3.1, .z = 3.2 },
            // }});


            // in.face_blendshapes->emplace_back({{.face_blendshapes = {}}});
        });
    });
});
