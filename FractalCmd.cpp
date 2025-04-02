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

void visualizeJuliaSet(const JuliaSet& juliaSet, int width, int height, double scale) {

    std::vector<std::vector<double>> values(height, std::vector<double>(width, 0.0));

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {

            double xPos = (x - width / 2.0) * scale;
            double yPos = (y - height / 2.0) * scale;

            VEC3F point(xPos, yPos, 0.0);

            double value = juliaSet.queryFieldValue(point);

            values[y][x] = value;
        }
    }

    double minVal = values[0][0];
    double maxVal = values[0][0];
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            minVal = std::min(minVal, values[y][x]);
            maxVal = std::max(maxVal, values[y][x]);
        }
    }

    std::ostringstream oss;
    oss << "Julia Set Visualization (" << width << "x" << height << "):\n";

    oss << "+";
    for (int x = 0; x < width; x++) {
        oss << "-";
    }
    oss << "+\n";

    for (int y = 0; y < height; y++) {
        oss << "|";
        for (int x = 0; x < width; x++) {

            double normalized = (values[y][x] - minVal) / (maxVal - minVal);


            const char* chars = " .,;:+*#";
            int numChars = 8;
            int index = (int)(normalized * (numChars - 1));
            index = std::max(0, std::min(numChars - 1, index));

            oss << chars[index];
        }
        oss << "|\n";
    }

    oss << "+";
    for (int x = 0; x < width; x++) {
        oss << "-";
    }
    oss << "+\n";

    MGlobal::displayInfo(oss.str().c_str());

    oss.str("");
    oss << "Value range: [" << minVal << ", " << maxVal << "]";
    MGlobal::displayInfo(oss.str().c_str());
}

void visualizeVersorField(const VersorMap& versorMap, int width, int height, double scale) {
    MGlobal::displayInfo("Versor Field Visualization (" + MString() + width + "x" + height + "):");

    // Top border
    std::string topBorder = "+";
    for (int x = 0; x < width; x++) {
        topBorder += "-";
    }
    topBorder += "+";
    MGlobal::displayInfo(topBorder.c_str());

    const char* directions = "^/>v\\<";

    // Generate and output one line at a time
    for (int y = 0; y < height; y++) {
        std::string line = "|";

        for (int x = 0; x < width; x++) {
            double xPos = (x - width / 2.0) * scale;
            double yPos = (y - height / 2.0) * scale;

            VEC3F point(xPos, yPos, 0.0);

            VEC3F fieldVector = versorMap.getFieldValue(point);

            double magnitude = fieldVector.norm();

            if (magnitude < 1e-6) {
                line += " ";
                continue;
            }

            double angle = atan2(fieldVector[1], fieldVector[0]);

            // Map angle to 6 directions (^, /, >, \, v, <)
            // Convert angle from [-pi, pi] to [0, 2pi]
            double normalizedAngle = angle;
            if (normalizedAngle < 0) normalizedAngle += 2 * M_PI;

            int index = static_cast<int>(normalizedAngle / (2 * M_PI / 6)) % 6;

            line += directions[index];
        }

        line += "|";
        MGlobal::displayInfo(line.c_str());
    }


    MGlobal::displayInfo(topBorder.c_str());

    std::ostringstream stats;

    VEC3F center(0, 0, 0);
    VEC3F samplePoint(scale, scale, 0);

    VEC3F centerValue = versorMap.getFieldValue(center);
    VEC3F sampleValue = versorMap.getFieldValue(samplePoint);

    stats << "Field samples: ";
    stats << "Center: (" << centerValue[0] << ", " << centerValue[1] << ", " << centerValue[2] << "), ";
    stats << "Sample: (" << sampleValue[0] << ", " << sampleValue[1] << ", " << sampleValue[2] << ")";

    MGlobal::displayInfo(stats.str().c_str());
}

PortalMap extractPortalsFromMesh(const MFnMesh& mayaMesh) {
    // This function extracts portal information from a Maya mesh
    // as described in the design document section 2.1.1 Algorithm Details

    PortalMap portalMap;

    // Get mesh components
    MPointArray points;
    mayaMesh.getPoints(points, MSpace::kObject);

    double maxDistSq = 0.0;
    MPoint center(0, 0, 0);

    // Calculate center of the mesh
    for (unsigned int i = 0; i < points.length(); i++) {
        center += points[i];
    }
    if (points.length() > 0) {
        center = center / points.length();
    }

    // Find maximum distance from center
    for (unsigned int i = 0; i < points.length(); i++) {
        double distSq = (points[i] - center).length();
        if (distSq > maxDistSq) {
            maxDistSq = distSq;
        }
    }

    const int numPortals = 3;

    for (int i = 0; i < numPortals; i++) {
        double angle = 2.0 * M_PI * (i / static_cast<double>(numPortals));
        double radius = sqrt(maxDistSq) * 0.8;

        VEC3F portalCenter(
            center.x + radius * cos(angle),
            center.y + radius * sin(angle),
            center.z + 0.1 * i 
        );

        // Create AngleAxis rotation 
        AngleAxis<Real> rotation(angle + M_PI / 2, VEC3F(0, 1, 0));

        // Add portal to the map
        portalMap.addPortal(portalCenter, rotation);
    }

    portalMap.portalRadius = sqrt(maxDistSq) * 0.2;
    portalMap.portalScale = 0.8;

    return portalMap;
}



MStatus FractalCmd::doIt(const MArgList& args)
{
	// message in scriptor editor
	MGlobal::displayInfo("FractalCmd");





    int maxIterations = 5;
    double escapeRadius = 4.0;
    double cx = 0.0, cy = 0.5, cz = 0.0, cw = 0.0;
    int width = 50; 
    int height = 25; 
    double scale = 0.05;




    /*
    std::ostringstream oss;
    oss << "Julia Set Parameters:" << std::endl;
    oss << "  Iterations: " << maxIterations << std::endl;
    oss << "  Escape Radius: " << escapeRadius << std::endl;
    oss << "  c = (" << cx << ", " << cy << ", " << cz << ", " << cw << ")" << std::endl;
    MGlobal::displayInfo(oss.str().c_str());

    visualizeJuliaSet(juliaSet, width, height, scale);

    visualizeVersorField(versorMap, width, height, scale);
    */








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

    // Read portal data and construct PortalMap instance
    // PortalMap pm = (...);
    PortalMap portalMap = extractPortalsFromMesh(mayaMesh);

    // Display portal map info
    std::ostringstream portalInfo;
    portalInfo << "Extracted " << portalMap.portalCenters.size() << " portals from mesh:" << std::endl;
    for (size_t i = 0; i < portalMap.portalCenters.size(); i++) {
        const VEC3F& center = portalMap.portalCenters[i];
        portalInfo << "  Portal " << i << ": Center ("
            << center[0] << ", " << center[1] << ", " << center[2] << ")" << std::endl;
        portalInfo << "  Rotation angle: " << portalMap.portalRotations[i].angle() << std::endl;
    }
    portalInfo << "Portal radius: " << portalMap.portalRadius << std::endl;
    portalInfo << "Portal scale: " << portalMap.portalScale << std::endl;

    MGlobal::displayInfo(portalInfo.str().c_str());


    // Construct Julia Set with the PortalMap
    QUATERNION juliaC(cw, cx, cy, cz);
    JuliaSet juliaSet(maxIterations, escapeRadius, juliaC);

    juliaSet.setPortalMap(portalMap);

    // Display Julia Set parameters
    std::ostringstream oss;
    oss << "Julia Set Parameters:" << std::endl;
    oss << "  Iterations: " << maxIterations << std::endl;
    oss << "  Escape Radius: " << escapeRadius << std::endl;
    oss << "  c = (" << cx << ", " << cy << ", " << cz << ", " << cw << ")" << std::endl;
    MGlobal::displayInfo(oss.str().c_str());

    // Visualize the Julia Set
    visualizeJuliaSet(juliaSet, width, height, scale);

    // Set up versor field
    Versor versor;
    Modulus modulus;
    VersorMap versorMap(versor, modulus);
    visualizeVersorField(versorMap, width, height, scale);


    // Generate fractal meshes

    // Perform marching cubes
    Mesh fractalMesh;
    MarchingCubes(fractalMesh, juliaSet);

    // Convert back to Maya MFnMesh
    MFnMesh outputMesh = fractalMesh.toMaya();
    //MFnMesh outputMesh = processedMesh.toMaya();
    //MFnMesh outputMesh = inputMesh.toMaya();


    // should we add a check here?
    /*
    if (outputMesh == MObject::kNullObj) {
        MGlobal::displayError("Failed to create output mesh");
        return MStatus::kFailure;
    }
    */


    // Print confirmation
    MGlobal::displayInfo("Fractal processing completed for mesh: " + meshName);

    return MStatus::kSuccess;
}

