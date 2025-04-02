#pragma once

#include "Quaternion/SETTINGS.h"
#include "Quaternion/QUATERNION.h"
#include <vector>

#include "PortalMap.h"
#include "VersorMap.h"

class JuliaSet {
public:
	JuliaSet(int maxIter = 10, double maxMag = 4.0, const QUATERNION& c = QUATERNION(0.0, 0.5, 0.0, 0.0));

	// Returns whether the point is in the Julia set (false)
	Real queryFieldValue(const VEC3F& point, double escapeRadius = 4.0) const;

	// Iteration func
	QUATERNION applyIteration(const QUATERNION& point) const;



	void setQuaternionC(const QUATERNION& newC);
	void setMaxIterations(int maxIter);
	void setMaxMagnitude(double maxMag);

	void setPortalMap(const PortalMap& map) { field = map; }
	const PortalMap& getPortalMap() const { return field; }

private:
	int maxIterations;
	double maxMagnitude;
	QUATERNION c;
	PortalMap field;
};