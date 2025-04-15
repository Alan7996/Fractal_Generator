#pragma once

#include "Quaternion/SETTINGS.h"
#include "Quaternion/QUATERNION.h"
#include <vector>

#include "PortalMap.h"
#include "VersorMap.h"
#include "mesh.h"

class JuliaSet {
public:
	JuliaSet(int maxIter, double maxMag, double alpha_, double beta_, const QUATERNION& c, Versor versor);

	// Returns whether the point is in the Julia set (false)
	Real queryFieldValue(const VEC3F& point, double escapeRadius = 4.0, size_t idx = 0, size_t num_iter = 1) const;

	// Iteration func
	QUATERNION applyIteration(const QUATERNION& point) const;

	void setInputMesh(const Mesh& mesh);
	VEC3F computeClosestPointOnMesh(const VEC3F& point, size_t idx, size_t num_iter) const;
	bool isPointInsideMesh(const VEC3F& point) const;
	Real computeSignedDistanceToMesh(const VEC3F& point, size_t idx, size_t num_iter) const;

	void setQuaternionC(const QUATERNION& newC);
	void setMaxIterations(int maxIter);
	void setMaxMagnitude(double maxMag);

	void setPortalMap(const PortalMap& map) { pm = map; }
	const PortalMap& getPortalMap() const { return pm; }

	PortalMap pm;

private:
	int maxIterations;
	double maxMagnitude;
	QUATERNION c;
	Versor noise;

	Mesh inputMesh;
	bool hasMesh = false;

	double boundary_threshold = 1.0;
	double scale_factor = 2.0;
	double alpha, beta;
};