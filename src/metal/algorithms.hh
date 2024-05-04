#pragma once

#include <simd/simd.h>
#include <vector>

std::vector<simd::float3> calculateNormals(std::vector<unsigned> vcount, std::vector<unsigned> fxyz, std::vector<float> xyz);
std::vector<uint16_t> triangles(std::vector<unsigned> vcount, std::vector<unsigned> fxyz);