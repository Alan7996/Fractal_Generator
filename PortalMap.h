#pragma once

#include <vector>

#include "Quaternion/SETTINGS.h"

class PortalMap {
public:
    PortalMap()
        : portalRadius(0.3), portalScale(0.5) {};

    PortalMap(const std::vector<VEC3F>& centers,
        const std::vector<AngleAxis<Real>>& rotations,
        Real radius = 0.3,
        Real scale = 0.5)
        : portalCenters(centers),
        portalRotations(rotations),
        portalRadius(radius),
        portalScale(scale) {
        if (portalRotations.size() != portalCenters.size()) {
            portalRotations.resize(portalCenters.size(), AngleAxis<Real>(0, VEC3F(0, 1, 0))); // Default no rotation
        }
    }

    VEC3F getFieldValue(const VEC3F& pos) const;

    void addPortal(const VEC3F& center, const AngleAxis<Real>& rotation) {
        portalCenters.push_back(center);
        portalRotations.push_back(rotation);
    }

    bool hasPortals() const { return !portalCenters.empty(); }

    const std::vector<VEC3F>& getPortalCenters() const { return portalCenters; }
    
    Real getPortalRadius() const { return portalRadius; }
    
    Real getPortalScale() const { return portalScale; }

private:
    std::vector<VEC3F>           portalCenters;
    std::vector<AngleAxis<Real>> portalRotations;
    Real  portalRadius;
    Real  portalScale;
};