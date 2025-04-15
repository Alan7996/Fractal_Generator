#pragma once
#include <cmath>
#include "Quaternion/SETTINGS.h"
#include "PerlinNoise/PerlinNoise.h"

class Versor {
public:
    siv::PerlinNoise nx, ny, nz;
    double scale = 1.0;
    unsigned int octave = 1u;

    Versor();
    Versor(unsigned int seedNx, unsigned int seedNy, unsigned int seedNz, unsigned int octave_, double scale_): octave(octave_), scale(scale_) {
        nx.reseed(seedNx);
        ny.reseed(seedNy);
        nz.reseed(seedNz);
    }
    
    VEC3F getFieldValue(const VEC3F& pos) const;
};

class Modulus {
public:
    const double alpha = 2.0;
    const double beta = 0.1;
    double radius = 1.0;
    VEC3F getFieldValue(const VEC3F& pos) const;
};
