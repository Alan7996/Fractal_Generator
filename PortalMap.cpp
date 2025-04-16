#include "PortalMap.h"
#include "vec.h"

PortalMap::PortalMap() {
}

PortalMap::PortalMap(double sx, double sy, double sz, double tx, double ty, double tz, double rx, double ry, double rz) {
    //portalRadius = radius;
    MAT4 scaleMat, rotMat, transMat;
    setScaleMat(&scaleMat, sx, sy, sz);
    setRotMat(&rotMat, rx, ry, rz);
    setTransMat(&transMat, tx, ty, tz);
    createTransformMat(scaleMat, rotMat, transMat);
}

void PortalMap::setScaleMat(MAT4* mat, double sx, double sy, double sz) {
    mat->col(0) << sx, 0.0, 0.0, 0.0;
    mat->col(1) << 0.0, sy, 0.0, 0.0;
    mat->col(2) << 0.0, 0.0, sz, 0.0;
    mat->col(3) << 0.0, 0.0, 0.0, 1.0;
}

void PortalMap::setRotMat(MAT4* mat, double rx, double ry, double rz) {
    double radX = rx * M_PI / 180.0;
    double radY = ry * M_PI / 180.0;
    double radZ = rz * M_PI / 180.0;

    // Create individual rotation matrices
    Eigen::Matrix3d rotX;
    rotX = Eigen::AngleAxisd(radX, Eigen::Vector3d::UnitX());

    Eigen::Matrix3d rotY;
    rotY = Eigen::AngleAxisd(radY, Eigen::Vector3d::UnitY());

    Eigen::Matrix3d rotZ;
    rotZ = Eigen::AngleAxisd(radZ, Eigen::Vector3d::UnitZ());

    // Combine them: ZYX order
    Eigen::Matrix3d rotation = rotZ * rotY * rotX;

    // Convert to 4x4 matrix
    *mat = MAT4::Identity();
    mat->block<3, 3>(0, 0) = rotation;
}

// need to confirm about the col major mat
void PortalMap::setTransMat(MAT4* mat, double tx, double ty, double tz) {
    *mat = MAT4::Identity();
    mat->col(3) << tx, ty, tz, 1.0;
}

void PortalMap::createTransformMat(MAT4 scaleMat, MAT4 rotMat, MAT4 transMat) {
    TransformMats transforms = {scaleMat, rotMat, transMat, transMat * rotMat * scaleMat};
    portalTransforms.emplace_back(transforms);
}

void PortalMap::addPortal(double tx, double ty, double tz, double rx, double ry, double rz, double sx, double sy, double sz) {
    MAT4 scaleMat, rotMat, transMat;
    setScaleMat(&scaleMat, sx, sy, sz);
    setRotMat(&rotMat, rx, ry, rz);
    setTransMat(&transMat, tx, ty, tz);
    createTransformMat(scaleMat, rotMat, transMat);
}

void PortalMap::setTransformMat(size_t idx, MAT4 scaleMat, MAT4 rotMat, MAT4 transMat) {
    portalTransforms[idx].scaleMat = scaleMat;
    portalTransforms[idx].rotMat = rotMat;
    portalTransforms[idx].transMat = transMat;
    portalTransforms[idx].transformMat = transMat * rotMat * scaleMat;
}

VEC3F PortalMap::getFieldValue(const VEC3F& pos, size_t idx, size_t num_iter) const {
    VEC4F posWorld;
    posWorld << pos[0], pos[1], pos[2], 1.0;

    MAT4 trans = portalTransforms[idx].transformMat;
    for (size_t i = 1; i < num_iter; ++i) {
        trans *= trans;
    }

    VEC4F posNew = trans * posWorld;

    VEC3F posReturn;
    posReturn << posNew[0], posNew[1], posNew[2];

    return posReturn;
}

VEC3F PortalMap::getInvFieldValue(const VEC3F& pos, size_t idx, size_t num_iter) const {
    VEC4F posWorld;
    posWorld << pos[0], pos[1], pos[2], 1.0;

    MAT4 trans = portalTransforms[idx].transformMat.inverse();
    for (size_t i = 1; i < num_iter; ++i) {
        trans *= trans;
    }
    VEC4F posNew = trans * posWorld;

    VEC3F posReturn;
    posReturn << posNew[0], posNew[1], posNew[2];

    return posReturn;
}

