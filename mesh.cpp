#include "mesh.h"

#include <maya/MItMeshPolygon.h>
#include <maya/MPointArray.h>
#include <maya/MFloatVectorArray.h>
#include <maya/MGlobal.h>
#include <maya/MIntArray.h>
#include <maya/MStatus.h>
#include <maya/MTypes.h>
#include <vector>

void Mesh::fromMaya(const MFnMesh& mayaMesh) {
    minVert = VEC3F(std::numeric_limits<double>::max(), std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
    maxVert = VEC3F(std::numeric_limits<double>::min(), std::numeric_limits<double>::min(), std::numeric_limits<double>::min());

    // Retrieve vertices from the Maya mesh
    MPointArray points_;
    mayaMesh.getPoints(points_, MSpace::kObject);
    for (unsigned int i = 0; i < points_.length(); ++i) {
        const MPoint& p = points_[i];
        vertices.push_back(VEC3F(static_cast<float>(p.x),
                                      static_cast<float>(p.y),
                                      static_cast<float>(p.z)));
        minVert = VEC3F(std::min(minVert[0], static_cast<double>(p.x)),
                        std::min(minVert[1], static_cast<double>(p.y)),
                        std::min(minVert[2], static_cast<double>(p.z)));
        maxVert = VEC3F(std::max(maxVert[0], static_cast<double>(p.x)),
                        std::max(maxVert[1], static_cast<double>(p.y)),
                        std::max(maxVert[2], static_cast<double>(p.z)));
    }

    // Retrieve normals from the Maya mesh
    MFloatVectorArray normals_;
    mayaMesh.getNormals(normals_, MSpace::kObject);
    for (unsigned int i = 0; i < normals_.length(); ++i) {
        const MFloatVector& n = normals_[i];
        normals.push_back(VEC3F(n.x, n.y, n.z));
    }

    // Retrieve material assignment
    MObjectArray shaders;
    MIntArray indices_;
    mayaMesh.getConnectedShaders(0, shaders, indices_);
    if (shaders.length() > 0) {
        material = shaders[0];  // Assuming single material
    }

    // Iterate over each polygon and perform fan triangulation to generate triangle indices
    MItMeshPolygon polyIter(mayaMesh.object());
    for (; !polyIter.isDone(); polyIter.next()) {
        MIntArray vertexList;
        polyIter.getVertices(vertexList);

        // Skip degenerate polygons
        if (vertexList.length() < 3) continue;

        // Fan triangulation: for a polygon with vertices v0, v1, ... vn-1,
        // create triangles (v0, v1, v2), (v0, v2, v3), ..., (v0, vn-2, vn-1)
        for (unsigned int i = 1; i < vertexList.length() - 1; ++i) {
            indices.push_back(static_cast<unsigned int>(vertexList[0]));
            indices.push_back(static_cast<unsigned int>(vertexList[i]));
            indices.push_back(static_cast<unsigned int>(vertexList[i + 1]));
        }
    }
}

MObject Mesh::toMaya() const {
    MStatus status;

    // Create an MPointArray from our vertices
    MPointArray points;
    for (size_t i = 0; i < vertices.size(); ++i) {
        const VEC3F& v = vertices[i];
        points.append(MPoint(static_cast<double>(v[0]),
                             static_cast<double>(v[1]),
                             static_cast<double>(v[2])));
    }

    // Compute the number of faces
    unsigned int numFaces = (unsigned int)indices.size() / 3;
    MIntArray faceCounts;
    MIntArray faceConnects;

    // For each triangle, set the face count to 3 and add the three vertex indices
    for (unsigned int i = 0; i < numFaces; ++i) {
        faceCounts.append(3);
        faceConnects.append(static_cast<int>(indices[3 * i]));
        faceConnects.append(static_cast<int>(indices[3 * i + 1]));
        faceConnects.append(static_cast<int>(indices[3 * i + 2]));
    }

    // Create a new Maya mesh using the points, face counts, and connectivity arrays
    MFnMesh fnMesh;
    MObject meshObj = fnMesh.create(points.length(), numFaces, points, faceCounts, faceConnects, MObject::kNullObj, &status);
    if (status != MS::kSuccess) {
        return MObject::kNullObj;
    }

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