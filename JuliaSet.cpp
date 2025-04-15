#include "JuliaSet.h"
#include <cmath>

JuliaSet::JuliaSet(unsigned int maxIter = 10u, double maxMag = 4.0, double alpha_ = 1.0, double beta_ = 0.0, const QUATERNION& c = QUATERNION(0.0, 0.5, 0.0, 0.0), Versor versor = Versor())
    : maxIterations(maxIter), maxMagnitude(maxMag), alpha(alpha_), beta(beta_), c(c), noise(versor) {
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

VEC3F JuliaSet::computeClosestPointOnMesh(const VEC3F& point, size_t idx, size_t num_iter) const {
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


        VEC3F v1T, v2T, v3T;

        v1T = pm.getInvFieldValue(v1, idx, num_iter);
        v2T = pm.getInvFieldValue(v2, idx, num_iter);
        v3T = pm.getInvFieldValue(v3, idx, num_iter);


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
    VEC3F origin = point + rayDir * 1e-3f;   // Offset origin slightly

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


Real JuliaSet::computeSignedDistanceToMesh(const VEC3F& point, size_t idx, size_t num_iter) const {
    if (!hasMesh) return 0.0;

    // First, compute the closest point on the mesh
    VEC3F closestPoint = computeClosestPointOnMesh(point, idx, num_iter);
    Real unsignedDistance = (point - closestPoint).norm();

    // Determine if point is inside using ray casting
    bool isInside = isPointInsideMesh(point);

    return isInside ? unsignedDistance : -unsignedDistance;
}

Real JuliaSet::queryFieldValue(const VEC3F& point, double escapeRadius, size_t idx, size_t num_iter) const {
    // Perturb the current position by perlin noise. Approximately simulating 
    // perturbed mesh surface without actually editing the mesh.
    VEC3F currPos = point + alpha * noise.getFieldValue(point + VEC3F(beta, beta, beta));
    // Calculate signed distance to mesh
    return computeSignedDistanceToMesh(currPos, idx, num_iter);
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