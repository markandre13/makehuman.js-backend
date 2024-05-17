#include "algorithms.hh"
#include <print>

using namespace std;

vector<simd::float3> calculateNormals(vector<unsigned> vcount, vector<unsigned> fxyz, vector<float> xyz) {
    vector<simd::float3> nv(xyz.size() / 3, simd::make_float3(0, 0, 0));
    vector<unsigned> nc(xyz.size() / 3, 0);

    size_t ifxyz = 0;
    for (auto count : vcount) {
        auto i0 = fxyz[ifxyz] * 3;
        auto i1 = fxyz[ifxyz + 1] * 3;
        auto i2 = fxyz[ifxyz + 2] * 3;

        // println("triangle {} {} {} in {}", i0, i1, i2, xyz.size());

        auto v0 = simd::make_float3(xyz[i0], xyz[i0 + 1], xyz[i0 + 2]);
        auto v1 = simd::make_float3(xyz[i1], xyz[i1 + 1], xyz[i1 + 2]);
        auto v2 = simd::make_float3(xyz[i2], xyz[i2 + 1], xyz[i2 + 2]);
        auto u = v1 - v0;
        auto v = v2 - v0;
        auto n = simd::normalize(simd::cross(u, v));

        for (auto i = 0; i < count; ++i) {
            auto idx = fxyz[ifxyz];
            nv[idx] += n;
            ++nc[idx];
            ++ifxyz;
        }
    }

    for (size_t i = 0; i < nv.size(); ++i) {
        nv[i] =  nv[i] / nc[i];
    }

    return nv;
}

vector<uint16_t> triangles(vector<unsigned> vcount, vector<unsigned> fxyz) {
    vector<uint16_t> fout;
    size_t ifxyz = 0;
    for (auto count : vcount) {
        switch(count) {
            case 3:
                fout.push_back(fxyz[ifxyz++]);
                fout.push_back(fxyz[ifxyz++]);
                fout.push_back(fxyz[ifxyz++]);
                break;
            case 4: {
                auto i0 = fxyz[ifxyz++];
                auto i1 = fxyz[ifxyz++];
                auto i2 = fxyz[ifxyz++];
                auto i3 = fxyz[ifxyz++];
                fout.push_back(i0);
                fout.push_back(i1);
                fout.push_back(i2);
                fout.push_back(i0);
                fout.push_back(i2);
                fout.push_back(i3);
            } break;
            default:
                throw runtime_error("can not triangulate face with more than 4 edges");
        }
    }
    return fout;
}

namespace math {

simd::float4x4 makePerspective(float fovRadians, float aspect, float znear, float zfar) {
    using simd::float4;
    float ys = 1.f / tanf(fovRadians * 0.5f);
    float xs = ys / aspect;
    float zs = zfar / (znear - zfar);
    return simd_matrix_from_rows((float4){xs, 0.0f, 0.0f, 0.0f}, (float4){0.0f, ys, 0.0f, 0.0f}, (float4){0.0f, 0.0f, zs, znear * zs}, (float4){0, 0, -1, 0});
}

simd::float4x4 makeXRotate(float angleRadians) {
    using simd::float4;
    const float a = angleRadians;
    return simd_matrix_from_rows((float4){1.0f, 0.0f, 0.0f, 0.0f}, (float4){0.0f, cosf(a), sinf(a), 0.0f}, (float4){0.0f, -sinf(a), cosf(a), 0.0f},
                                 (float4){0.0f, 0.0f, 0.0f, 1.0f});
}

simd::float4x4 makeYRotate(float angleRadians) {
    using simd::float4;
    const float a = angleRadians;
    return simd_matrix_from_rows((float4){cosf(a), 0.0f, sinf(a), 0.0f}, (float4){0.0f, 1.0f, 0.0f, 0.0f}, (float4){-sinf(a), 0.0f, cosf(a), 0.0f},
                                 (float4){0.0f, 0.0f, 0.0f, 1.0f});
}

simd::float4x4 makeZRotate(float angleRadians) {
    using simd::float4;
    const float a = angleRadians;
    return simd_matrix_from_rows((float4){cosf(a), sinf(a), 0.0f, 0.0f}, (float4){-sinf(a), cosf(a), 0.0f, 0.0f}, (float4){0.0f, 0.0f, 1.0f, 0.0f},
                                 (float4){0.0f, 0.0f, 0.0f, 1.0f});
}

simd::float4x4 makeTranslate(const simd::float3& v) {
    using simd::float4;
    const float4 col0 = {1.0f, 0.0f, 0.0f, 0.0f};
    const float4 col1 = {0.0f, 1.0f, 0.0f, 0.0f};
    const float4 col2 = {0.0f, 0.0f, 1.0f, 0.0f};
    const float4 col3 = {v.x, v.y, v.z, 1.0f};
    return simd_matrix(col0, col1, col2, col3);
}

simd::float4x4 makeScale(const simd::float3& v) {
    using simd::float4;
    return simd_matrix((float4){v.x, 0, 0, 0}, (float4){0, v.y, 0, 0}, (float4){0, 0, v.z, 0}, (float4){0, 0, 0, 1.0});
}

simd::float3x3 discardTranslation(const simd::float4x4& m) { return simd_matrix(m.columns[0].xyz, m.columns[1].xyz, m.columns[2].xyz); }

}  // namespace math