#include "VersorMap.h"
#include "PerlinNoise/PerlinNoise.h"


VEC3F Versor::getFieldValue(const VEC3F& pos) const {
    
    // not sure about seed
    siv::PerlinNoise perlin(11111);


    // to be adjusted
    const double scale = 0.1;

    double x = perlin.noise3D_01(pos[0] * scale, pos[1] * scale, pos[2] * scale);
    double y = perlin.noise3D_01(pos[0] * scale + 100.0, pos[1] * scale + 100.0, pos[2] * scale + 100.0);
    double z = perlin.noise3D_01(pos[0] * scale + 200.0, pos[1] * scale + 200.0, pos[2] * scale + 200.0);

    VEC3F noise(x * 2.0 - 1, y * 2.0 - 1, z * 2.0 - 1);


    double len = noise.norm();
    if (len > Epsilon) {
        noise /= len;
    }
    else {
        
        noise = VEC3F(1.0, 0.0, 0.0);
    }

    
    
    return noise;
}

VEC3F Modulus::getFieldValue(const VEC3F& pos) const {
    

    double sdf = pos.norm() - radius;

    double modulationFactor = std::exp(-alpha * std::abs(sdf)) + beta;


    double spatialVariation = 1.0 + 0.2 * sin(pos[0] * 3.0) * sin(pos[1] * 3.0) * sin(pos[2] * 3.0);
    modulationFactor *= spatialVariation;
    
    
    
    return modulationFactor * pos;
}

VEC3F VersorMap::getFieldValue(const VEC3F& pos) const {
    return (versor.getFieldValue(pos)).cwiseProduct(modulus.getFieldValue(pos));
}