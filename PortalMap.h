#pragma once

#include <vector>

#include "Quaternion/SETTINGS.h"

class PortalMap {
private:
    MAT4 scaleMat = MAT4::Identity();
    MAT4 rotMat = MAT4::Identity();
    MAT4 tranMat = MAT4::Identity();

public:
    PortalMap();
    PortalMap(double sx, double sy, double sz, double tx, double ty, double tz, double rx, double ry, double rz);
    
    VEC3F getFieldValue(const VEC3F& pos) const;

    VEC3F getInvFieldValue(const VEC3F& pos) const;

    void setTranMat(double tx, double ty, double tz);
    void setTransformMat();
    void setScaleMat(double sx, double sy, double sz);
    void setRotMat(double rx, double ry, double rz);

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

    MAT4 getScaleMat() const { return scaleMat; }
    MAT4 getRotMat() const { return rotMat; }
    MAT4 getTranMat() const { return tranMat; }

    MAT4 getTransformMat() const { return TransformMat; }


    std::vector<VEC3F>           portalCenters;
    std::vector<AngleAxis<Real>> portalRotations;
    //double  portalRadius;
    //double  portalScale;

    MAT4 TransformMat = MAT4::Identity();


};