#pragma once

#include <vector>

#include "Quaternion/SETTINGS.h"

typedef struct {
    MAT4 scaleMat;
    MAT4 rotMat;
    MAT4 transMat;
    MAT4 transformMat;
} TransformMats;

class PortalMap {
public:
    PortalMap();
    PortalMap(double sx, double sy, double sz, double tx, double ty, double tz, double rx, double ry, double rz);
    
    VEC3F getFieldValue(const VEC3F& pos, size_t idx, size_t num_iter) const;

    VEC3F getInvFieldValue(const VEC3F& pos, size_t idx, size_t num_iter) const;

    void setTransMat(MAT4* mat, double tx, double ty, double tz);
    void setScaleMat(MAT4* mat, double sx, double sy, double sz);
    void setRotMat(MAT4* mat, double rx, double ry, double rz);
    void setTransformMat(size_t idx, MAT4 scaleMat, MAT4 rotMat, MAT4 transMat);
    void createTransformMat(MAT4 scaleMat, MAT4 rotMat, MAT4 transMat);

    /*
    void addPortal(const VEC3F& center, const AngleAxis<Real>& rotation) {
        portalCenters.push_back(center);
        portalRotations.push_back(rotation);
    }

    void PortalMap::clearPortals() {
        portalCenters.clear();
        portalRotations.clear();
    }

    bool hasPortals() const { return !portalCenters.empty(); }

    const std::vector<VEC3F>& getPortalCenters() const { return portalCenters; }
    
    Real getPortalRadius() const { return portalRadius; }
    
    Real getPortalScale() const { return portalScale; }
    */

    MAT4 getScaleMat(size_t idx) const { return portalTransforms[idx].scaleMat; }
    MAT4 getRotMat(size_t idx) const { return portalTransforms[idx].rotMat; }
    MAT4 getTransMat(size_t idx) const { return portalTransforms[idx].transMat; }

    MAT4 getTransformMat(size_t idx) const { return portalTransforms[idx].transformMat; }


    std::vector<VEC3F>          portalCenters;
    std::vector<TransformMats>  portalTransforms;
    std::vector<double>         portalRadius;
    // std::vector<double>  portalScale;


};