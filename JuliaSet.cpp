#include "JuliaSet.h"
#include <cmath>

JuliaSet::JuliaSet(int maxIter, double maxMag, const QUATERNION& c)
    : maxIterations(maxIter), maxMagnitude(maxMag), c(c) {
}

bool JuliaSet::isPointInSet(const VEC3F& point, double escapeRadius) const {

    QUATERNION z(0.0, point[0], point[1], point[2]);


    for (int i = 0; i < maxIterations; i++) {
        z.juliaIteration(c);

        double magSquared = z.magnitude();
        if (magSquared > escapeRadius * escapeRadius) {
            return false;
        }
    }

    return true;
}


QUATERNION JuliaSet::applyIteration(const QUATERNION& point) const {
    QUATERNION result = point;
    result.juliaIteration(c);
    return result;
}



void JuliaSet::setQuaternionC(const QUATERNION& newC) {
    c = newC;
}

void JuliaSet::setMaxIterations(int maxIter) {
    maxIterations = maxIter;
}

void JuliaSet::setMaxMagnitude(double maxMag) {
    maxMagnitude = maxMag;
}