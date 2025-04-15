#include "VersorMap.h"

VEC3F Versor::getFieldValue(const VEC3F& pos) const {
    
    VEC3F p = pos * scale;

    VEC3F v(
        nx.octave3D_01(p.x(), p.y(), p.z(), octave) * 2.0 - 1.0,
        ny.octave3D_01(p.x(), p.y(), p.z(), octave) * 2.0 - 1.0,
        nz.octave3D_01(p.x(), p.y(), p.z(), octave) * 2.0 - 1.0
    );

    return v.normalized();
}

VEC3F Modulus::getFieldValue(const VEC3F& pos) const {
    
    double sdf = pos.norm() - radius;
    double modulationFactor = std::exp(-alpha * std::abs(sdf)) + beta;
    double spatialVariation = 1.0 + 0.2 * sin(pos[0] * 3.0) * sin(pos[1] * 3.0) * sin(pos[2] * 3.0);
    modulationFactor *= spatialVariation;
    
    return modulationFactor * pos;
}
