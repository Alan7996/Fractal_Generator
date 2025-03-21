#pragma once

#include "Quaternion/SETTINGS.h"

// TODO, need to query versor and modulus field map values for the input position
class Versor {
public:
    VEC3F getFieldValue(const VEC3F& pos) const;
};

class Modulus {
public:
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