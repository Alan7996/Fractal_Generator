#pragma once

#include <vector>

#include "Quaternion/SETTINGS.h"

class PortalMap {
public:
    PortalMap() {};

    VEC3F getFieldValue(const VEC3F& pos) const;

private:
    std::vector<VEC3F>           portalCenters;
    std::vector<AngleAxis<Real>> portalRotations;
    Real  portalRadius;
    Real  portalScale;
};