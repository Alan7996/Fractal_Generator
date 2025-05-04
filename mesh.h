#pragma once

#include "Quaternion/SETTINGS.h"
#include <maya/MFnMesh.h>
#include "PortalMap.h"

// custom indexed mesh representation
class Mesh {
public:
	std::vector<VEC3F> vertices;
	std::vector<VEC3F> normals;
	std::vector<uint> indices;
	MObject material;
	VEC3F minVert;
	VEC3F maxVert;

	// Store original Maya mesh and UV set name for copying
	MString uvSetName;
	std::vector<float> uvU, uvV;
	std::vector<int> uvCountsVec, uvIdsVec;
	std::vector<VEC3F> originalVertices;
	
	void fromMaya(const MFnMesh& mayaMesh);
	MObject toMaya() const;
    void fromMesh(const Mesh& other); // Helper to copy UV and original vertices
};