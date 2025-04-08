#include "PortalMap.h"
#include "vec.h"

PortalMap::PortalMap() {
    //portalRadius = 0.2;
    //portalScale = 0.8;
    setScaleMat(0.75, 1.0, 1.0);
    setRotMat(0, 90, 0);
    setTranMat(0.0, 0.0, 1.0);
    setTransformMat();

}

PortalMap::PortalMap(double sx, double sy, double sz, double tx, double ty, double tz, double rx, double ry, double rz) {
    //portalRadius = radius;
    setScaleMat(sx, sy, sz);
    setRotMat(rx, ry, rz);
    setTranMat(tx, ty, tz);
    setTransformMat();
}

void PortalMap::setScaleMat(double sx, double sy, double sz) {
    scaleMat.col(0) << sx, 0.0, 0.0, 0.0;
    scaleMat.col(1) << 0.0, sy, 0.0, 0.0;
    scaleMat.col(2) << 0.0, 0.0, sz, 0.0;
    scaleMat.col(3) << 0.0, 0.0, 0.0, 1.0;

}

void PortalMap::setRotMat(double rx, double ry, double rz) {
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
    rotMat = MAT4::Identity();
    rotMat.block<3, 3>(0, 0) = rotation;
}

// need to confirm about the col major mat
void PortalMap::setTranMat(double tx, double ty, double tz) {
    tranMat = MAT4::Identity();
    tranMat.col(3) << tx, ty, tz, 1.0;
}

void PortalMap::setTransformMat() {
    TransformMat = tranMat * rotMat * scaleMat;
}

VEC3F PortalMap::getFieldValue(const VEC3F& pos) const {
    VEC4F posWorld;
    posWorld << pos[0], pos[1], pos[2], 1.0;


    VEC4F posNew = TransformMat * posWorld;

    VEC3F posReturn;
    posReturn << posNew[0], posNew[1], posNew[2];

    return posReturn;
}

VEC3F PortalMap::getInvFieldValue(const VEC3F& pos) const {
    VEC4F posWorld;
    posWorld << pos[0], pos[1], pos[2], 1.0;

    VEC4F posNew = TransformMat.inverse() * posWorld;

    VEC3F posReturn;
    posReturn << posNew[0], posNew[1], posNew[2];

    return posReturn;
}

