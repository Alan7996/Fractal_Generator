#include "JuliaSet.h"
#include <cmath>

JuliaSet::JuliaSet(int maxIter, double maxMag, const QUATERNION& c)
    : maxIterations(maxIter), maxMagnitude(maxMag), c(c) {
    field = PortalMap();
}

Real JuliaSet::queryFieldValue(const VEC3F& point, double escapeRadius) const {

    VEC3F currPos = point;
    Real currMag = currPos.norm();
    int numIter = 0;

    while (currMag < maxMagnitude && numIter < maxIterations) {
        VEC3F newPos = field.getFieldValue(currPos);
        currPos = newPos;
        currMag = currPos.norm();
        ++numIter;
    }

    return log(currMag);
}

// input output vex 
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