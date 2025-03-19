#pragma once

#include "Quaternion/SETTINGS.h"
#include "Quaternion/QUATERNION.h"
#include <vector>

class JuliaSet {
public:
	JuliaSet(int maxIter = 10, double maxMag = 4.0, const QUATERNION& c = QUATERNION(0.0, 0.5, 0.0, 0.0));

	// Returns whether the point is in the Julia set (false)
	bool isPointInSet(const VEC3F& point, double escapeRadius = 4.0) const;

	// Iteration func
	QUATERNION applyIteration(const QUATERNION& point) const;



	void setQuaternionC(const QUATERNION& newC);
	void setMaxIterations(int maxIter);
	void setMaxMagnitude(double maxMag);

private:
	int maxIterations;
	double maxMagnitude;
	QUATERNION c;
};