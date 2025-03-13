#include "mesh.h"

#include <maya/MItMeshPolygon.h>
#include <maya/MPointArray.h>
#include <maya/MFloatVectorArray.h>
#include <maya/MIntArray.h>
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

    // Iterate over each polygon and perform fan triangulation to generate triangle indices.
    MItMeshPolygon polyIter(mayaMesh.object());
    for (; !polyIter.isDone(); polyIter.next()) {
        MIntArray vertexList;
        polyIter.getVertices(vertexList);

        // Skip degenerate polygons
        if (vertexList.length() < 3)
            continue;

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
