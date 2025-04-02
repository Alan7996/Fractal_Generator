#pragma once

#include <vector>

#include "Quaternion/SETTINGS.h"

class PortalMap {
public:
    PortalMap();
    PortalMap(double radius, double scale);

    VEC3F getFieldValue(const VEC3F& pos) const;

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


    std::vector<VEC3F>           portalCenters;
    std::vector<AngleAxis<Real>> portalRotations;
    double  portalRadius;
    double  portalScale;
};