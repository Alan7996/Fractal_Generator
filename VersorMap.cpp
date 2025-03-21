#include "VersorMap.h"

VEC3F Versor::getFieldValue(const VEC3F& pos) const {
    return pos; // TODO
}

VEC3F Modulus::getFieldValue(const VEC3F& pos) const {
    return pos; // TODO
}

VEC3F VersorMap::getFieldValue(const VEC3F& pos) const {
    return (versor.getFieldValue(pos)).cwiseProduct(modulus.getFieldValue(pos));
}