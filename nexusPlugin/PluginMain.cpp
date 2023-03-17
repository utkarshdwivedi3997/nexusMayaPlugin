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

#include "NexusCommand.h"
#include "NexusSolverNode.h"
#include "NexusClothNode.h"

MStatus initializePlugin(MObject obj)
{
    MStatus   status = MStatus::kSuccess;
    MFnPlugin plugin(obj, "Nexus Physics", "1.0", "Any");

    // Register Command
    status = plugin.registerCommand("nexus", NexusCommand::creator, NexusCommand::newSyntax);
    if (!status) {
        status.perror("registerCommand");
        return status;
    }

    // Register Nodes
    status = plugin.registerNode("nexusSolverNode", NexusSolverNode::id,
        NexusSolverNode::creator, NexusSolverNode::initialize);
    if (!status)
    {
        status.perror("registerNode NexusSolverNode");
        return status;
    }

    status = plugin.registerNode("nexusClothNode", NexusClothNode::id,
        NexusClothNode::creator, NexusClothNode::initialize);
    if (!status)
    {
        status.perror("registerNode NexusClothNode");
        return status;
    }

    // Load Mel GUI files
    char buffer[2048];
    MString pluginPath = plugin.loadPath();
    MString menuPath = MString("source \"") + pluginPath + MString("/nexusMenu.mel\"");
    sprintf_s(buffer, 2048, menuPath.asChar(), pluginPath);
    MGlobal::executeCommand(buffer, true);

    return status;
}

MStatus uninitializePlugin(MObject obj)
{
    MStatus   status = MStatus::kSuccess;
    MFnPlugin plugin(obj);

    status = plugin.deregisterCommand("nexus");
    if (!status) {
        status.perror("deregisterCommand");
        return status;
    }

    status = plugin.deregisterNode(NexusSolverNode::id);
    if (!status)
    {
        status.perror("deregisterNode NexusSolverNode");
        return status;
    }

    status = plugin.deregisterNode(NexusClothNode::id);
    if (!status)
    {
        status.perror("deregisterNode NexusClothNode");
        return status;
    }

    return status;
}