#include "JuliaSet.h"
#include <cmath>

JuliaSet::JuliaSet(int maxIter, double maxMag, const QUATERNION& c)
    : maxIterations(maxIter), maxMagnitude(maxMag), c(c) {
    pm = PortalMap();
}


void JuliaSet::setInputMesh(const Mesh& mesh) {
    // Store the input mesh for distance field calculations
    inputMesh = mesh;
    hasMesh = true;
}

// Helper function: compute closest point on triangle ABC to point P
static VEC3F closestPointOnTriangle(const VEC3F& p, const VEC3F& a, const VEC3F& b, const VEC3F& c) {
    // Compute edges
    VEC3F ab = b - a;
    VEC3F ac = c - a;
    VEC3F ap = p - a;

    // Compute dot products
    Real d1 = ab.dot(ap);
    Real d2 = ac.dot(ap);

    // Check if P in vertex region outside A
    if (d1 <= 0 && d2 <= 0) return a;

    // Check if P in vertex region outside B
    VEC3F bp = p - b;
    Real d3 = ab.dot(bp);
    Real d4 = ac.dot(bp);
    if (d3 >= 0 && d4 <= d3) return b;

    // Check if P in edge region of AB
    Real vc = d1 * d4 - d3 * d2;
    if (vc <= 0 && d1 >= 0 && d3 <= 0) {
        Real v = d1 / (d1 - d3);
        return a + ab * v;
    }

    // Check if P in vertex region outside C
    VEC3F cp = p - c;
    Real d5 = ab.dot(cp);
    Real d6 = ac.dot(cp);
    if (d6 >= 0 && d5 <= d6) return c;

    // Check if P in edge region of AC
    Real vb = d5 * d2 - d1 * d6;
    if (vb <= 0 && d2 >= 0 && d6 <= 0) {
        Real w = d2 / (d2 - d6);
        return a + ac * w;
    }

    // Check if P in edge region of BC
    Real va = d3 * d6 - d5 * d4;
    if (va <= 0 && (d4 - d3) >= 0 && (d5 - d6) >= 0) {
        Real w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
        return b + (c - b) * w;
    }

    // P inside face region. Compute barycentric coordinates (u, v, w)
    Real denom = 1.0 / (va + vb + vc);
    Real v = vb * denom;
    Real w = vc * denom;
    return a + ab * v + ac * w;
}

VEC3F JuliaSet::computeClosestPointOnMesh(const VEC3F& point) const {
    if (!hasMesh) return VEC3F(); // Return (0,0,0) if no mesh is available

    VEC3F closestPoint;
    Real minDistance = std::numeric_limits<Real>::max();

    for (size_t i = 0; i < inputMesh.indices.size(); i += 3) {
        unsigned int idx1 = inputMesh.indices[i];
        unsigned int idx2 = inputMesh.indices[i + 1];
        unsigned int idx3 = inputMesh.indices[i + 2];

        const VEC3F& v1 = inputMesh.vertices[idx1];
        const VEC3F& v2 = inputMesh.vertices[idx2];
        const VEC3F& v3 = inputMesh.vertices[idx3];

        VEC4F v14, v24, v34;
        v14 << v1[0], v1[1], v1[2], 1.0;
        v24 << v2[0], v2[1], v2[2], 1.0;
        v34 << v3[0], v3[1], v3[2], 1.0;

        VEC4F v14T = pm.getTransformMat() * v14;
        VEC4F v24T = pm.getTransformMat() * v24;
        VEC4F v34T = pm.getTransformMat() * v34;

        VEC3F v1T, v2T, v3T;
        v1T << v14T[0], v14T[1], v14T[2];
        v2T << v24T[0], v24T[1], v24T[2];
        v3T << v34T[0], v34T[1], v34T[2];


        VEC3F candidate = closestPointOnTriangle(point, v1T, v2T, v3T);
        Real distance = (candidate - point).norm();

        if (distance < minDistance) {
            minDistance = distance;
            closestPoint = candidate;
        }
    }

    return closestPoint;
}

bool rayIntersectsTriangle(const VEC3F& origin, const VEC3F& dir,
    const VEC3F& v0, const VEC3F& v1, const VEC3F& v2) {
    const float EPSILON = 1e-6f;

    VEC3F edge1 = v1 - v0;
    VEC3F edge2 = v2 - v0;
    VEC3F h = dir.cross(edge2);
    float a = edge1.dot(h);

    if (a > -EPSILON && a < EPSILON)
        return false; // Ray is parallel

    float f = 1.0 / a;
    VEC3F s = origin - v0;
    float u = f * s.dot(h);
    if (u < 0.0 || u > 1.0)
        return false;

    VEC3F q = s.cross(edge1);
    float v = f * dir.dot(q);
    if (v < 0.0 || u + v > 1.0)
        return false;

    float t = f * edge2.dot(q);
    if (t > EPSILON)
        return true;

    return false;
}

bool JuliaSet::isPointInsideMesh(const VEC3F& point) const {
    VEC3F rayDir = VEC3F(1.0f, 0.5f, 0.25f); // Avoid axis-aligned
    rayDir.normalize();
    VEC3F origin = point + rayDir * 1e-4f;   // Offset origin slightly

    int intersectionCount = 0;

    for (size_t i = 0; i < inputMesh.indices.size(); i += 3) {
        const VEC3F& v0 = inputMesh.vertices[inputMesh.indices[i]];
        const VEC3F& v1 = inputMesh.vertices[inputMesh.indices[i + 1]];
        const VEC3F& v2 = inputMesh.vertices[inputMesh.indices[i + 2]];

        if (rayIntersectsTriangle(origin, rayDir, v0, v1, v2)) {
            intersectionCount++;
        }
    }

    return (intersectionCount % 2) == 1;
}


Real JuliaSet::computeSignedDistanceToMesh(const VEC3F& point) const {
    if (!hasMesh) return 0.0;

    // First, compute the closest point on the mesh
    VEC3F closestPoint = computeClosestPointOnMesh(point);
    Real unsignedDistance = (point - closestPoint).norm();

    // Determine if point is inside using ray casting
    bool isInside = isPointInsideMesh(point);

    return isInside ? unsignedDistance : -unsignedDistance;
}

Real JuliaSet::queryFieldValue(const VEC3F& point, double escapeRadius) const {

    VEC3F currPos = point;
    Real currMag = currPos.norm();
    int numIter = 0;
    // Compute distance to mesh (if available)
    VEC3F closestPoint = computeClosestPointOnMesh(point);

    // Blend the distance field with the Julia set
    Real blendFactor = 1.0;

    while (currMag < maxMagnitude && numIter < maxIterations) {
        VEC3F newPos = pm.getFieldValue(currPos);
        currPos = newPos;
        currMag = currPos.norm();
        ++numIter;
    }

    // Calculate signed distance to mesh
    Real distanceValue = computeSignedDistanceToMesh(point);

    // Return normalized value ensuring values span across 0.5
    return distanceValue;
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