#pragma once

#include <simd/simd.h>

#include <cmath>
#include <vector>

std::vector<simd::float3> calculateNormals(std::vector<unsigned> vcount, std::vector<unsigned> fxyz, std::vector<float> xyz);
std::vector<uint16_t> triangles(std::vector<unsigned> vcount, std::vector<unsigned> fxyz);
template <class T>
inline bool isZero(T a) {
    return fabsf(a) < std::numeric_limits<T>::epsilon();
}

namespace math {

constexpr simd::float3 add(const simd::float3& a, const simd::float3& b) { return {a.x + b.x, a.y + b.y, a.z + b.z}; }

constexpr simd_float4x4 makeIdentity() {
    using simd::float4;
    return (simd_float4x4){(float4){1.f, 0.f, 0.f, 0.f}, (float4){0.f, 1.f, 0.f, 0.f}, (float4){0.f, 0.f, 1.f, 0.f}, (float4){0.f, 0.f, 0.f, 1.f}};
}

simd::float4x4 makePerspective(float fovRadians, float aspect, float znear, float zfar);
simd::float4x4 makeXRotate(float angleRadians);
simd::float4x4 makeYRotate(float angleRadians);
simd::float4x4 makeZRotate(float angleRadians);
simd::float4x4 makeTranslate(const simd::float3& v);
simd::float4x4 makeScale(const simd::float3& v);
simd::float3x3 discardTranslation(const simd::float4x4& m);
}  // namespace math
