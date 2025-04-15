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

    VEC3F minBox, maxBox;
    minBox = inputMesh.minVert - VEC3F(alpha, alpha, alpha);
    maxBox = inputMesh.maxVert + VEC3F(alpha, alpha, alpha);
    
    for (size_t portalIdx = 0; portalIdx < juliaSet.pm.portalTransforms.size(); ++portalIdx) {
        VEC4F minBoxIter, maxBoxIter;
        for (int i = 1; i <= maxIterations; ++i) {
            if (i == 1) {
                minBox = juliaSet.pm.getFieldValue(minBox, portalIdx, i);
                maxBox = juliaSet.pm.getFieldValue(maxBox, portalIdx, i);

                minBoxIter << minBox[0], minBox[1], minBox[2], 1.0;
                maxBoxIter << maxBox[0], maxBox[1], maxBox[2], 1.0;
            
                MarchingCubes(fractalMesh, juliaSet, minBox, maxBox, portalIdx, i);
                MFnMesh outputMesh = fractalMesh.toMaya();
                continue;
            }

            // Apply the transformation matrix iteratively
            minBoxIter = juliaSet.pm.getTransformMat(portalIdx) * minBoxIter;
            maxBoxIter = juliaSet.pm.getTransformMat(portalIdx) * maxBoxIter;

            MarchingCubes(fractalMesh, juliaSet, {minBoxIter[0], minBoxIter[1], minBoxIter[2]}, {maxBoxIter[0], maxBoxIter[1], maxBoxIter[2]}, portalIdx, i);
            MFnMesh outputMesh = fractalMesh.toMaya();
        }
    }

    // Print confirmation
    MGlobal::displayInfo("Fractal processing completed for mesh: " + meshName);

    return MStatus::kSuccess;
}

