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
        ////////////////////////////////////////////////////////////////////////
        // Global declarations and initialization
        ////////////////////////////////////////////////////////////////////////
        global int $nodeCounter;
        global string $nodesContainer;
        $nodeCounter = 1;

        ////////////////////////////////////////////////////////////////////////
        // Procedure to create a single fractal node UI group.
        // The unique nodeID is appended to each control's name.
        ////////////////////////////////////////////////////////////////////////
        global proc createFractalNodeFields(string $nodeID) {
            // Create a frameLayout for this fractal node.
            frameLayout -label ("Fractal Node " + $nodeID) -collapsable true -marginWidth 10 -marginHeight 10 ("nodeFrame_" + $nodeID);
                columnLayout -adjustableColumn true -columnAlign "left";
                    // Selected Object Field for this node.
                    textFieldButtonGrp -columnAlign3 "left" "left" "left" 
                        -label "Selected Object: " 
                        -buttonLabel "Select" 
                        -bc "updateSelectionField" 
                        ("mySelectionField_" + $nodeID);

                    string $btnName = "generateButton_" + $nodeID;
                    button
                        -label "Generate Portal"
                        -command ("createPortalCube \"" + $nodeID + "\"")
                        $btnName;

                    // Generated portal mesh name
                    textFieldGrp
                        -label "Portal Mesh:"
                        -columnAlign2 "left" "left"
                        -enable false
                        -text ""
                        ("generatedMeshField_" + $nodeID);

                    // Noise Scale Slider
                    floatSliderGrp -label "Noise Scale" -field true -minValue 0 -maxValue 1.0 -value 0.05 -step 0.0001 -precision 4 -columnAlign3 "left" "left" "left" ("myAlphaSlider_" + $nodeID);
                    // Noise Offset Slider
                    floatSliderGrp -label "Noise Offset" -field true -minValue 0 -maxValue 10 -value 0.0 -step 0.01 -precision 2 -columnAlign3 "left" "left" "left" ("myBetaSlider_" + $nodeID);
                    // Versor Scale Slider
                    floatSliderGrp -label "Versor Scale" -field true -minValue 0 -maxValue 10 -value 9.0 -step 0.01 -precision 2 -columnAlign3 "left" "left" "left" ("myVersorScaleSlider_" + $nodeID);
                    // Versor Octave Slider
                    intSliderGrp -label "Versor Octave" -field true -minValue 0 -maxValue 8 -value 1 -columnAlign3 "left" "left" "left" ("myVersorOctaveSlider_" + $nodeID);
                    // Iterations Slider
                    intSliderGrp -label "Iterations" -field true -minValue 1 -maxValue 8 -value 2 -columnAlign3 "left" "left" "left" ("myNumIterationSlider_" + $nodeID);
                    
                    // RowLayout for Delete Button (centered)
                    rowLayout -numberOfColumns 1 -columnAlign1 "center";
                        button -label "Delete Fractal Node" -command ("deleteFractalNode \"" + $nodeID + "\"");
                    setParent ..;
                setParent ..; // End columnLayout
            setParent ..; // End frameLayout
        }

        ////////////////////////////////////////////////////////////////
        // Procedure to create a cube named "portalCube_<nodeID>"
        ////////////////////////////////////////////////////////////////
        global proc createPortalCube(string $nodeID) {
            string $node[] = `polyCube -w 1 -h 1 -d 1 -name ("portalCube_" + $nodeID)`;
            string $name = $node[0];
            // turn on shape-override template (wireframe-only) display
            setAttr ($name + ".overrideEnabled")      1;
            setAttr ($name + ".overrideDisplayType")  1;

            string $fieldName = ("generatedMeshField_" + $nodeID);
            if (`control -exists $fieldName`) {
                textFieldGrp -e -text $name $fieldName;
            }

            string $btn = "generateButton_" + $nodeID;
            if (`control -exists $btn`) {
                button -e -enable false $btn;
            } else {
                warning("Couldn¡¯t find button: " + $btn);
            }

            print("Created portal: " + $name + "\n");
        }

        ////////////////////////////////////////////////////////////////////////
        // Procedure to add a new fractal node group.
        ////////////////////////////////////////////////////////////////////////
        global proc addFractalNode() {
            global int $nodeCounter;
            global string $nodesContainer;
            string $nodeID = $nodeCounter;
            // Set parent to the nodes container so the new node's controls are added there.
            setParent $nodesContainer;
            createFractalNodeFields($nodeID);
            $nodeCounter++;
            setParent ..;  // Return to the previous layout.
        }

        ////////////////////////////////////////////////////////////////////////
        // Procedure to update the selection field.
        // This updates the selection field for the most recently added node.
        ////////////////////////////////////////////////////////////////////////
        global proc updateSelectionField() {
            global int $nodeCounter;
            string $selection[] = `ls -sl`;
            if (size($selection) > 0) {
                string $currentField = ("mySelectionField_" + ($nodeCounter - 1));
                textFieldButtonGrp -e -text $selection[0] $currentField;
            } else {
                warning "No object selected. Please select an object.";
            }
        }

        ////////////////////////////////////////////////////////////////////////
        // New procedure to delete a fractal node UI group.
        ////////////////////////////////////////////////////////////////////////
        global proc deleteFractalNode(string $nodeID) {
            // Delete the frameLayout for the specified fractal node.
            string $frameName = ("nodeFrame_" + $nodeID);
            if (`control -exists $frameName`) {
                deleteUI $frameName;
            }
        }

        ////////////////////////////////////////////////////////////////////////
        // Generate callback that processes each fractal node group.
        ////////////////////////////////////////////////////////////////////////
        global proc onGeneratePressed() {
            global int $nodeCounter;
            int $numNodes = $nodeCounter - 1;
            for ($i = 1; $i <= $numNodes; $i++) {
                string $selField = "mySelectionField_" + $i;

                // If the control does not exist (node deleted), skip it.
                if (!`control -exists $selField`) {
                    continue;
                }

                string $selObj = `textFieldButtonGrp -q -text ("mySelectionField_" + $i)`;
                string $alphaField = "myAlphaSlider_" + $i;
                string $betaField = "myBetaSlider_" + $i;
                string $versorScaleField = "myVersorScaleSlider_" + $i;
                string $versorOctaveField = "myVersorOctaveSlider_" + $i;
                string $numIterationField = "myNumIterationSlider_" + $i;

                string $portalName = ("portalCube_" + $i);
                string $selectedObject = `textFieldButtonGrp -q -text $selField`;
                float $t[] = `xform -q -ws -translation $portalName`;
                float $r[] = `xform -q -ws -rotation    $portalName`;
                float $s[] = `xform -q -os -scale       $portalName`;
                float $posX = $t[0];
                float $posY = $t[1];
                float $posZ = $t[2];
                float $rotX = $r[0];
                float $rotY = $r[1];
                float $rotZ = $r[2];
                float $scaleX = $s[0] / 2.0;
                float $scaleY = $s[1] / 2.0;
                float $scaleZ = $s[2] / 2.0;
                float $alpha = `floatSliderGrp -q -value $alphaField`;
                float $beta = `floatSliderGrp -q -value $betaField`;
                float $versorScale = `floatSliderGrp -q -value $versorScaleField`;
                int $versorOctave = `intSliderGrp -q -value $versorOctaveField`;
                int $numIterations = `intSliderGrp -q -value $numIterationField`;
                
                string $cmd = ("FractalCmd \"" + $selectedObject + "\" " 
                               + $posX + " " + $posY + " " + $posZ + " " 
                               + $rotX + " " + $rotY + " " + $rotZ + " " 
                               + $scaleX + " " + $scaleY + " " + $scaleZ + " " 
                               + $alpha + " " + $beta + " " + $versorScale + " " 
                               + $versorOctave + " " + $numIterations);
                print ("Executing for node " + $i + ": " + $cmd + "\n");
                eval($cmd);
            }
        }

        ////////////////////////////////////////////////////////////////////////
        // Main UI window procedure.
        ////////////////////////////////////////////////////////////////////////
        global proc openSelectionWindow() {
            if (`window -exists mySelectionWindow`) {
                deleteUI mySelectionWindow;
            }
            window -title "Fractal Generator" -resizeToFitChildren true mySelectionWindow;
            columnLayout -adjustableColumn true;

            button -label "Add fractal node" -command "addFractalNode";
            scrollLayout -width 270 -height 500 -horizontalScrollBarThickness 16 -verticalScrollBarThickness 16;
                global string $nodesContainer;
            $nodesContainer = `columnLayout -adjustableColumn true`;
            setParent ..;

            button -label "Generate" -command "onGeneratePressed";
            showWindow mySelectionWindow;
        }
    )";

    *status = MGlobal::executeCommand(melCreateSelectionUI);
    if (*status != MS::kSuccess) {
        cerr << "Error executing melCreateSelectionUI!" << endl;
    }

    // Create a menu item in Maya¡¯s main window to open the UI.
    MGlobal::executeCommand(R"(
        global string $gMainWindow;
        if (`menu -exists fractalPluginMenu`) {
            deleteUI -menu fractalPluginMenu;
        }
        setParent $gMainWindow;
        menu -label "Fractal Plugin" -tearOff true -parent $gMainWindow fractalPluginMenu;
        menuItem -label "Open Selection UI" -command "openSelectionWindow" -parent fractalPluginMenu;
    )");
}

MStatus initializePlugin(MObject obj)
{
    MStatus status = MStatus::kSuccess;
    MFnPlugin plugin(obj, "MyPlugin", "1.0", "Any");

    // Register your command. Ensure FractalCmd::creator is correctly implemented.
    status = plugin.registerCommand("FractalCmd", FractalCmd::creator);
    if (!status) {
        status.perror("registerCommand");
        return status;
    }

    // Create the UI.
    createSelectionUI(&status);
    if (!status) {
        status.perror("createSelectionUI");
        return status;
    }

    return status;
}

MStatus uninitializePlugin(MObject obj)
{
    MStatus status = MStatus::kSuccess;
    MFnPlugin plugin(obj);

    status = plugin.deregisterCommand("FractalCmd");
    if (!status) {
        status.perror("deregisterCommand");
        return status;
    }

    MGlobal::executeCommand(R"(
        deleteUI fractalPluginMenu;
    )");

    return status;
}
