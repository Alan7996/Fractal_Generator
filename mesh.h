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
	
	void fromMaya(const MFnMesh& mayaMesh);
	MObject toMaya() const;
	void copyMaterial(const MObject material_) { material = material_; };
};