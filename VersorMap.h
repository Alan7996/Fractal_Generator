#pragma once
#include <cmath>
#include "Quaternion/SETTINGS.h"

// TODO, need to query versor and modulus field map values for the input position
class Versor {
public:
    const double Epsilon = 1e-10;
    VEC3F getFieldValue(const VEC3F& pos) const;
};

class Modulus {
public:
    const double alpha = 2.0;
    const double beta = 0.1;
    double radius = 1.0;
    VEC3F getFieldValue(const VEC3F& pos) const;
};

class VersorMap {
public:
    VersorMap(Versor versor_, Modulus modulus_) : versor(versor_), modulus(modulus_) {};

    VEC3F getFieldValue(const VEC3F& pos) const;

private:
    Versor versor;
    Modulus modulus;
};