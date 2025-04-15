#include "FractalCmd.h"

#include <maya/MGlobal.h>
#include <maya/MArgList.h>
#include <maya/MFnMesh.h>
#include <maya/MSelectionList.h>
#include <maya/MDagPath.h>
#include <maya/MPointArray.h>
#include <list>

#include "JuliaSet.h"
#include "MarchingCubes.h"
#include "mesh.h"
#include "PortalMap.h"

#define BBOX_SIZE 8

FractalCmd::FractalCmd() : MPxCommand()
{
}

FractalCmd::~FractalCmd()
{
}

PortalMap extractPortalsFromMesh(const MFnMesh& mayaMesh) {
    // This function extracts portal information from a Maya mesh
    // as described in the design document section 2.1.1 Algorithm Details

    PortalMap portalMap;

    // Get mesh components
    MPointArray points;
    mayaMesh.getPoints(points, MSpace::kObject);

    return portalMap;
}

MStatus FractalCmd::doIt(const MArgList& args)
{
	// message in scriptor editor
	MGlobal::displayInfo("FractalCmd");

    int maxIterations = 2;
    double escapeRadius = 4.0;
    double cx = 0.0, cy = 0.5, cz = 0.0, cw = 0.0;
    int width = 50; 
    int height = 25; 
    double scale = 0.05;
    double alpha = 0.05;
    double beta = 0.0;
    unsigned int versorOctave = 1u;
    double versorScale = 9.0;

	if (args.length() != 1) {
        MGlobal::displayError("FractalCmd requires a single argument: the name of the selected mesh.");
        return MStatus::kFailure;
    }

    MString meshName = args.asString(0);
    MSelectionList selection;
    MDagPath dagPath;

    if (selection.add(meshName) != MStatus::kSuccess ||
        selection.getDagPath(0, dagPath) != MStatus::kSuccess) {
        MGlobal::displayError("Failed to retrieve the specified mesh: " + meshName);
        return MStatus::kFailure;
    }

    MFnMesh mayaMesh(dagPath);

    // Convert input MFnMesh to custom mesh class
    Mesh inputMesh;
    inputMesh.fromMaya(mayaMesh);

    // Set up versor field. The seed values are from the authors.
    Versor versor = Versor(83888u, 39388u, 17474u, versorOctave, versorScale);

    // Construct Julia Set with the PortalMap
    PortalMap portalMap = PortalMap();
    QUATERNION juliaC(cw, cx, cy, cz);
    JuliaSet juliaSet(maxIterations, escapeRadius, alpha, beta, juliaC, versor);

    juliaSet.setInputMesh(inputMesh);
    juliaSet.setPortalMap(portalMap);

    // Perform marching cubes
    Mesh fractalMesh;

    auto getBboxMinMax = [](VEC3F bbox[BBOX_SIZE], VEC3F* minVert, VEC3F* maxVert) {
        *minVert = bbox[0];
        *maxVert = bbox[0];
        for (size_t i = 1; i < BBOX_SIZE; ++i) {
            *minVert = VEC3F(min((*minVert)[0], bbox[i][0]), min((*minVert)[1], bbox[i][1]), min((*minVert)[2], bbox[i][2]));
            *maxVert = VEC3F(max((*maxVert)[0], bbox[i][0]), max((*maxVert)[1], bbox[i][1]), max((*maxVert)[2], bbox[i][2]));
        }
    };

    VEC3F bbox[BBOX_SIZE] = {
        VEC3F(inputMesh.minVert[0] - alpha, inputMesh.minVert[1] - alpha, inputMesh.minVert[2] - alpha),
        VEC3F(inputMesh.minVert[0] - alpha, inputMesh.minVert[1] - alpha, inputMesh.maxVert[2] + alpha),
        VEC3F(inputMesh.minVert[0] - alpha, inputMesh.maxVert[1] + alpha, inputMesh.minVert[2] - alpha),
        VEC3F(inputMesh.maxVert[0] + alpha, inputMesh.minVert[1] - alpha, inputMesh.minVert[2] - alpha),
        VEC3F(inputMesh.minVert[0] - alpha, inputMesh.maxVert[1] + alpha, inputMesh.maxVert[2] + alpha),
        VEC3F(inputMesh.maxVert[0] + alpha, inputMesh.minVert[1] - alpha, inputMesh.maxVert[2] + alpha),
        VEC3F(inputMesh.maxVert[0] + alpha, inputMesh.maxVert[1] + alpha, inputMesh.minVert[2] - alpha),
        VEC3F(inputMesh.maxVert[0] + alpha, inputMesh.maxVert[1] + alpha, inputMesh.maxVert[2] + alpha)
    };

    VEC3F minBoxIter, maxBoxIter;
    VEC3F currBbox[8];
    for (size_t portalIdx = 0; portalIdx < juliaSet.pm.portalTransforms.size(); ++portalIdx) {
        for (int i = 1; i <= maxIterations; ++i) {
            if (i == 1) {
                for (size_t cornerIdx = 0; cornerIdx < BBOX_SIZE; ++cornerIdx) {
                    currBbox[cornerIdx] = juliaSet.pm.getFieldValue(bbox[cornerIdx], portalIdx, i);
                }
                getBboxMinMax(currBbox, &minBoxIter, &maxBoxIter);
            
                MarchingCubes(fractalMesh, juliaSet, minBoxIter, maxBoxIter, portalIdx, i);
                MFnMesh outputMesh = fractalMesh.toMaya();
                continue;
            }

            for (size_t cornerIdx = 0; cornerIdx < BBOX_SIZE; ++cornerIdx) {
                // Apply the transformation matrix iteratively through parameter i
                currBbox[cornerIdx] = juliaSet.pm.getFieldValue(bbox[cornerIdx], portalIdx, i);
            }
            getBboxMinMax(currBbox, &minBoxIter, &maxBoxIter);

            MarchingCubes(fractalMesh, juliaSet, {minBoxIter[0], minBoxIter[1], minBoxIter[2]}, {maxBoxIter[0], maxBoxIter[1], maxBoxIter[2]}, portalIdx, i);
            MFnMesh outputMesh = fractalMesh.toMaya();
        }
    }

    // Print confirmation
    MGlobal::displayInfo("Fractal processing completed for mesh: " + meshName);

    return MStatus::kSuccess;
}

