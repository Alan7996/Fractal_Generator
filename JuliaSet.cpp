#include "JuliaSet.h"
#include <cmath>

JuliaSet::JuliaSet(int maxIter, double maxMag, const QUATERNION& c)
    : maxIterations(maxIter), maxMagnitude(maxMag), c(c) {
    field = PortalMap();
}


void JuliaSet::setInputMesh(const Mesh& mesh) {
    // Store the input mesh for distance field calculations
    inputMesh = mesh;
    hasMesh = true;
}

Real JuliaSet::computeDistanceToMesh(const VEC3F& point) const {
    if (!hasMesh) return 0.0; // If no mesh is set, return 0

    // Compute approximate distance to the mesh
    Real minDistance = std::numeric_limits<Real>::max();

    // Compute distance to each triangle
    for (size_t i = 0; i < inputMesh.indices.size(); i += 3) {
        unsigned int idx1 = inputMesh.indices[i];
        unsigned int idx2 = inputMesh.indices[i + 1];
        unsigned int idx3 = inputMesh.indices[i + 2];

        const VEC3F& v1 = inputMesh.vertices[idx1];
        const VEC3F& v2 = inputMesh.vertices[idx2];
        const VEC3F& v3 = inputMesh.vertices[idx3];

        // Compute distance to triangle (this is an approximation)
        // A more accurate approach would compute the exact point-triangle distance
        Real d1 = (point - v1).norm();
        Real d2 = (point - v2).norm();
        Real d3 = (point - v3).norm();

        Real triangleDist = std::min(std::min(d1, d2), d3);
        minDistance = std::min(minDistance, triangleDist);
    }

    return minDistance;
}


Real JuliaSet::queryFieldValue(const VEC3F& point, double escapeRadius) const {

    VEC3F currPos = point;
    Real currMag = currPos.norm();
    int numIter = 0;
    // Compute distance to mesh (if available)
    Real meshDistance = hasMesh ? computeDistanceToMesh(point) : 0.0;

    // Blend the distance field with the Julia set
    Real blendFactor = 1.0;

    while (currMag < maxMagnitude && numIter < maxIterations) {
        VEC3F newPos = field.getFieldValue(currPos);
        currPos = newPos;
        currMag = currPos.norm();
        ++numIter;
    }

    Real juliaValue = 0.5 + (log(currMag) - boundary_threshold) / scale_factor;

    // Calculate distance to mesh if available
    Real distanceValue = 0.5;

    if (hasMesh) {
        // Find closest distance to any face in the mesh
        Real minDistance = std::numeric_limits<Real>::max();
        bool isInside = false;

        // Very simple distance approximation - find closest vertex
        for (const auto& vertex : inputMesh.vertices) {
            Real dist = (point - vertex).norm();
            minDistance = std::min(minDistance, dist);
        }

        // Normalize the distance field
        Real maxDist = 1.0; // Maximum expected distance
        Real normalizedDist = minDistance / maxDist;

        // Map to field value (0.5 at approximate surface)
        distanceValue = 0.5 + normalizedDist - 0.2;
    }

    // Blend values and ensure crossing the threshold
    Real blendedValue = juliaValue * (1.0 - blendFactor) + distanceValue * blendFactor;

    // Critical: Normalize to ensure we cross the 0.5 threshold
    Real minFieldVal = 0.3;
    Real maxFieldVal = 0.7;

    // Return normalized value ensuring values span across 0.5
    return minFieldVal + (blendedValue - juliaValue) / (1.0 - juliaValue) * (maxFieldVal - minFieldVal);
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