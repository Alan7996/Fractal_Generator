// Based on Marching Cubes implementation by Paul Bourke
// modified to fit better with C++'s standard library rather than C
// https://paulbourke.net/geometry/polygonise/

#pragma once

#include "JuliaSet.h"
#include "mesh.h"

void MarchingCubes(Mesh& mesh, JuliaSet& js, VEC3F minBox, VEC3F maxBox, size_t idx, size_t num_iter, bool isLowRes);