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
