#include "PortalMap.h"

PortalMap::PortalMap() {
    portalRadius = 0.2;
    portalScale = 0.8;
}

PortalMap::PortalMap(double radius, double scale) {
    portalRadius = radius;
    portalScale = scale;
}



VEC3F PortalMap::getFieldValue(const VEC3F& pos) const {
    if (portalCenters.empty()) {
        return pos;
    }

    // Check if the point is inside any portal
    for (size_t i = 0; i < portalCenters.size(); i++) {
        const VEC3F& center = portalCenters[i];

        VEC3F toPoint = pos - center;
        Real distance = toPoint.norm();

        if (distance < portalRadius) {
            Real depth = 1.0 - distance / portalRadius;

            VEC3F rotated = portalRotations[i] * toPoint;

            Real scaleFactor = 1.0 - (1.0 - portalScale) * depth;

            VEC3F transformed = center + rotated * scaleFactor;

            VEC3F result = pos * (1.0 - depth) + transformed * depth;

            return result;
        }
    }

    return pos;
}