#include "mesh.h"

#include <maya/MItMeshPolygon.h>
#include <maya/MPointArray.h>
#include <maya/MFloatVectorArray.h>
#include <maya/MGlobal.h>
#include <maya/MIntArray.h>
#include <maya/MStatus.h>
#include <maya/MTypes.h>

#include <maya/MFnDagNode.h>
#include <maya/MDagPath.h>
#include <maya/MDagPathArray.h>
#include <maya/MDGModifier.h>
#include <maya/MFnTransform.h>
#include <maya/MTransformationMatrix.h>
#include <maya/MMatrix.h>

#include <limits>
#include <vector>

void Mesh::fromMaya(const MFnMesh& mayaMesh) {
    // grab the shape’s DAG path, pop off the shape to get the transform
    MDagPath shapePath;
    {
        MFnDagNode fnDag(mayaMesh.object());
        fnDag.getPath(shapePath);
    }
    shapePath.pop();
    MFnTransform fnXform(shapePath);
    MTransformationMatrix txMat(fnXform.transformationMatrix());
    MVector translation = txMat.getTranslation(MSpace::kWorld);

    minVert = VEC3F(std::numeric_limits<double>::max(), std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
    maxVert = VEC3F(std::numeric_limits<double>::min(), std::numeric_limits<double>::min(), std::numeric_limits<double>::min());

    // Retrieve vertices from the Maya mesh
    MPointArray points_;
    mayaMesh.getPoints(points_, MSpace::kWorld);

    vertices.clear();
    vertices.reserve(points_.length());
    originalVertices.reserve(points_.length());

    for (unsigned int i = 0; i < points_.length(); ++i) {
        const MPoint& p = points_[i];
        double x = static_cast<double>(p.x - translation.x);
        double y = static_cast<double>(p.y - translation.y);
        double z = static_cast<double>(p.z - translation.z);

        vertices.emplace_back(x, y, z);
        originalVertices.emplace_back(p.x, p.y, p.z);

        minVert = VEC3F(
            std::min(minVert[0], x),
            std::min(minVert[1], y),
            std::min(minVert[2], z)
        );
        maxVert = VEC3F(
            std::max(maxVert[0], x),
            std::max(maxVert[1], y),
            std::max(maxVert[2], z)
        );
    }

    // Retrieve normals from the Maya mesh
    MFloatVectorArray normals_;
    mayaMesh.getNormals(normals_, MSpace::kObject);
    normals.clear();
    for (unsigned int i = 0; i < normals_.length(); ++i) {
        const MFloatVector& n = normals_[i];
        normals.push_back(VEC3F(n.x, n.y, n.z));
    }

    // Retrieve material assignment
    MObjectArray shaders;
    MIntArray indices_;
    mayaMesh.getConnectedShaders(0, shaders, indices_);
    if (shaders.length() > 0) {
        material = shaders[0];
    }

    // Iterate over each polygon and perform fan triangulation to generate triangle indices
    MItMeshPolygon polyIter(mayaMesh.object());
    indices.clear();
    for (; !polyIter.isDone(); polyIter.next()) {
        MIntArray vertexList;
        polyIter.getVertices(vertexList);
        if (vertexList.length() < 3) continue;
        for (unsigned int i = 1; i < vertexList.length() - 1; ++i) {
            indices.push_back(static_cast<unsigned int>(vertexList[0]));
            indices.push_back(static_cast<unsigned int>(vertexList[i]));
            indices.push_back(static_cast<unsigned int>(vertexList[i + 1]));
        }
    }

    // current UV set name
    mayaMesh.getCurrentUVSetName(uvSetName);

    // raw UV coordinate arrays
    MFloatArray uArray, vArray;
    mayaMesh.getUVs(uArray, vArray, &uvSetName);
    
    uvU.clear();
    uvV.clear();
    uvU.reserve(uArray.length());
    uvV.reserve(vArray.length());
    for (unsigned i = 0; i < uArray.length(); ++i) {
        uvU.push_back(uArray[i]);
        uvV.push_back(vArray[i]);
    }

    // face‑vertex UV assignment (counts + indices)
    MIntArray uvCountsArr, uvIdsArr;
    mayaMesh.getAssignedUVs(uvCountsArr, uvIdsArr, &uvSetName);

    uvCountsVec.clear();
    uvIdsVec.clear();
    uvCountsVec.reserve(uvCountsArr.length());
    uvIdsVec.reserve(uvIdsArr.length());
    for (unsigned i = 0; i < uvCountsArr.length(); ++i) {
        uvCountsVec.push_back(uvCountsArr[i]);
    }
    for (unsigned i = 0; i < uvIdsArr.length(); ++i) {
        uvIdsVec.push_back(uvIdsArr[i]);
    }
}

MObject Mesh::toMaya() const {
    MStatus status;

    // Create an MPointArray from our vertices
    MPointArray points;
    for (size_t i = 0; i < vertices.size(); ++i) {
        const VEC3F& v = vertices[i];
        points.append(MPoint(static_cast<double>(v[0]), static_cast<double>(v[1]), static_cast<double>(v[2])));
    }

    // Compute the number of faces
    unsigned int numFaces = static_cast<unsigned int>(indices.size()) / 3;
    MIntArray faceCounts;
    MIntArray faceConnects;
    for (unsigned int i = 0; i < numFaces; ++i) {
        faceCounts.append(3);
        faceConnects.append(static_cast<int>(indices[3 * i]));
        faceConnects.append(static_cast<int>(indices[3 * i + 1]));
        faceConnects.append(static_cast<int>(indices[3 * i + 2]));
    }

    // Create a new Maya mesh using the points, face counts, and connectivity arrays
    MFnMesh fnMesh;
    MObject meshObj = fnMesh.create(
        points.length(), numFaces, points, faceCounts, faceConnects, MObject::kNullObj, &status
    );
    if (status != MS::kSuccess) {
        return MObject::kNullObj;
    }
    
    MFloatArray newU, newV;
    unsigned newVtxCount = (unsigned)vertices.size();
    newU.setLength(newVtxCount);
    newV.setLength(newVtxCount);

    for (unsigned i = 0; i < newVtxCount; ++i) {
        // find nearest original vertex
        double bestDist2 = DBL_MAX;
        unsigned bestJ = 0;
        const auto& p = vertices[i];
        for (unsigned j = 0; j < originalVertices.size(); ++j) {
            double dx = p[0] - originalVertices[j][0];
            double dy = p[1] - originalVertices[j][1];
            double dz = p[2] - originalVertices[j][2];
            double d2 = dx*dx + dy*dy + dz*dz;
            if (d2 < bestDist2) {
                bestDist2 = d2;
                bestJ     = j;
            }
        }
        newU[i] = uvU[bestJ];
        newV[i] = uvV[bestJ];
    }

    // ——— 2) Create UV set on the new mesh ———
    MString setName = uvSetName.length() ? uvSetName : MString("map1");
    status = fnMesh.createUVSet(setName);

    // ——— 3) Push UV coords into Maya ———
    status = fnMesh.setUVs(newU, newV, &setName);

    // ——— 4) Assign those UVs per face‑vertex ———
    // reuse the same faceCounts/faceConnects you used for the geometry
    MIntArray countsArr, idsArr;
    countsArr.setLength((unsigned)faceCounts.length());
    idsArr   .setLength((unsigned)faceConnects.length());
    for (unsigned f = 0; f < faceCounts.length();    ++f) countsArr[f] = faceCounts[f];
    for (unsigned k = 0; k < faceConnects.length();  ++k) idsArr[k]    = faceConnects[k];

    status = fnMesh.assignUVs(countsArr, idsArr, &setName);

    // ——— 5) Make it current ———
    fnMesh.setCurrentUVSetName(setName);

    // Set the vertex normals if the mesh has one normal per vertex
    if (normals.size() == vertices.size()) {
        MVectorArray mNormals;
        for (size_t i = 0; i < normals.size(); ++i) {
            const VEC3F& n = normals[i];
            mNormals.append(MFloatVector(n[0], n[1], n[2]));
        }
        MIntArray vertexIndices;
        for (unsigned int i = 0; i < vertices.size(); ++i) {
            vertexIndices.append(i);
        }
        fnMesh.setVertexNormals(mNormals, vertexIndices);
    }

    // Assign material back
    if (!material.isNull()) {
        MString shadingGroup;
        MFnDependencyNode depNode(material);
        shadingGroup = depNode.name();
        MGlobal::executeCommand("sets -e -forceElement " + shadingGroup + " " + fnMesh.name());
    }

    return meshObj;
}

// Helper to copy UV and original vertex data from another mesh
void Mesh::fromMesh(const Mesh& other) {
    uvSetName = other.uvSetName;
    uvU = other.uvU;
    uvV = other.uvV;
    uvCountsVec = other.uvCountsVec;
    uvIdsVec = other.uvIdsVec;
    material = other.material;
}
