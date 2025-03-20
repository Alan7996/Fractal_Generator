#include <maya/MPxCommand.h>
#include <maya/MFnPlugin.h>
#include <maya/MIOStream.h>
#include <maya/MString.h>
#include <maya/MArgList.h>
#include <maya/MGlobal.h>
#include <maya/MSimple.h>
#include <maya/MDoubleArray.h>
#include <maya/MPoint.h>
#include <maya/MPointArray.h>
#include <maya/MFnNurbsCurve.h>
#include <maya/MDGModifier.h>
#include <maya/MPlugArray.h>
#include <maya/MVector.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MStringArray.h>
#include <list>

#include "FractalCmd.h"

void createSelectionUI(MStatus* status) {
    const char* melCreateSelectionUI = R"(
        global proc openSelectionWindow() {
            if (`window -exists mySelectionWindow`) {
                deleteUI mySelectionWindow;
            }
            
            window -title "Select Object" -widthHeight 300 100 mySelectionWindow;
            columnLayout -adjustableColumn true;
            textFieldButtonGrp -label "Selected Object: " -buttonLabel "Select" 
                -bc ("updateSelectionField") mySelectionField;

        button - label "Generate" - command ("onGeneratePressed");

        showWindow mySelectionWindow;
    }

    global proc updateSelectionField() {
        string $selection[] = `ls - sl`;
            if (size($selection) > 0) {
                textFieldButtonGrp - e - text $selection[0] mySelectionField;
            }
            else {
                warning "No object selected. Please select an object.";
            }
    }

    global proc onGeneratePressed() {
        string $selectedObject = `textFieldButtonGrp - q - text mySelectionField`;
        if ($selectedObject == "") {
            warning "No object selected. Please select an object before generating.";
        } else {
            string $cmd = "FractalCmd \"" + $selectedObject + "\"";
            print("Executing: " + $cmd + "\n");
            eval($cmd);
        }
    }
    )";

    *status = MGlobal::executeCommand(melCreateSelectionUI);
    if (*status != MS::kSuccess) {
        cerr << "Error executing melCreateSelectionUI!" << endl;
    }

    // Create a menu item that contains our createGUI button
    MGlobal::executeCommand(R"(
        global string $gMainWindow;
        if (`menu -exists fractalPluginMenu`) {
            deleteUI -menu fractalPluginMenu;
        }
        setParent $gMainWindow;
        menu -label "Fractal Plugin" -tearOff true -parent $gMainWindow fractalPluginMenu;
        menuItem 
            -label "Open Selection UI" 
            -command "openSelectionWindow"  
            -parent fractalPluginMenu;
    )");
}


MStatus initializePlugin( MObject obj )
{
    MStatus   status = MStatus::kSuccess;
    MFnPlugin plugin( obj, "MyPlugin", "1.0", "Any");

    // Register Command
    status = plugin.registerCommand( "FractalCmd", FractalCmd::creator );
    if (!status) {
        status.perror("registerCommand");
        return status;
    }

    createSelectionUI(&status);
    if (!status) {
        status.perror("createSelectionUI");
        return status;
    }

    return status;
}

MStatus uninitializePlugin( MObject obj)
{
    MStatus   status = MStatus::kSuccess;
    MFnPlugin plugin( obj );

    status = plugin.deregisterCommand( "FractalCmd" );
    if (!status) {
	    status.perror("deregisterCommand");
	    return status;
    }

    MGlobal::executeCommand(R"(
        deleteUI fractalPluginMenu;
    )");

    return status;
}


