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
            
            window -title "Fractal Generator" -resizeToFitChildren true mySelectionWindow;
            columnLayout -adjustableColumn true;
            textFieldButtonGrp -label "Selected Object: " -buttonLabel "Select" 
                -bc ("updateSelectionField") mySelectionField;
            
            // Alpha, Beta, Versor Scale sliders
            floatSliderGrp -label "Noise Scale" -field true -minValue 0 -maxValue 1.0 -value 0.05 -step 0.0001 -precision 4 myAlphaSlider;
            floatSliderGrp -label "Noise Offset" -field true -minValue 0 -maxValue 10 -value 0.0 -step 0.01 -precision 2 myBetaSlider;
            floatSliderGrp -label "Versor Scale" -field true -minValue 0 -maxValue 10 -value 9.0 -step 0.01 -precision 2 myVersorScaleSlider;

            // Versor Octave slider (unsigned int 0–8)
            intSliderGrp -label "Versor Octave" -field true -minValue 0 -maxValue 8 -value 1 myVersorOctaveSlider;

            intSliderGrp -label "Iterations" -field true -minValue 1 -maxValue 8 -value 2 myNumIterationSlider;

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
                float $alpha = `floatSliderGrp -q -value myAlphaSlider`;
                float $beta = `floatSliderGrp -q -value myBetaSlider`;
                float $versorScale = `floatSliderGrp -q -value myVersorScaleSlider`;
                int $versorOctave = `intSliderGrp -q -value myVersorOctaveSlider`;
                int $numIterations = `intSliderGrp -q -value myNumIterationSlider`;

                string $cmd = "FractalCmd \"" + $selectedObject + "\" " + $alpha + " " + $beta + " " + $versorScale + " " + $versorOctave + " " + $numIterations;
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


