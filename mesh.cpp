#include "mesh.h"

#include <maya/MItMeshPolygon.h>
#include <maya/MPointArray.h>
#include <maya/MFloatVectorArray.h>
#include <maya/MIntArray.h>
#include <maya/MStatus.h>
#include <maya/MTypes.h>
#include <vector>

Mesh Mesh::fromMaya(const MFnMesh& mayaMesh) {
    Mesh mesh;

    // Retrieve vertices from the Maya mesh
    MPointArray points;
    mayaMesh.getPoints(points, MSpace::kObject);
    for (unsigned int i = 0; i < points.length(); ++i) {
        const MPoint& p = points[i];
        mesh.vertices.push_back(VEC3F(static_cast<float>(p.x),
                                      static_cast<float>(p.y),
                                      static_cast<float>(p.z)));
    }

    // Retrieve normals from the Maya mesh
    MFloatVectorArray normals;
    mayaMesh.getNormals(normals, MSpace::kObject);
    for (unsigned int i = 0; i < normals.length(); ++i) {
        const MFloatVector& n = normals[i];
        mesh.normals.push_back(VEC3F(n.x, n.y, n.z));
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
            mesh.indices.push_back(static_cast<unsigned int>(vertexList[0]));
            mesh.indices.push_back(static_cast<unsigned int>(vertexList[i]));
            mesh.indices.push_back(static_cast<unsigned int>(vertexList[i + 1]));
        }
    }

    return mesh;
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

    return meshObj;
}