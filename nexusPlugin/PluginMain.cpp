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
#include "NexusRigidBodyNode.h"

MStatus initializePlugin(MObject obj)
{
    MStatus   status = MStatus::kSuccess;
    MFnPlugin plugin(obj, "NexusPhysics");

    // Register Command
    status = plugin.registerCommand("nexus", NexusCommand::creator);
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

    status = plugin.registerNode("nexusRigidBodyNode", NexusRigidBodyNode::id,
        NexusRigidBodyNode::creator, NexusRigidBodyNode::initialize);
    if (!status)
    {
        status.perror("registerNode NexusRigidBodyNode");
        return status;
    }

    // Load Mel GUI files
    //char buffer[2048];
    //MString pluginPath = plugin.loadPath();
    //MString menuPath = MString("source \"") + pluginPath + MString("/nexusMenu.mel\"");
    //sprintf_s(buffer, 2048, menuPath.asChar(), pluginPath);
    //MGlobal::executeCommand(buffer, true);

    // Load Python GUI files (we're switching to Python because it's so much better than Mel)
    char buffer[2048];
    MString pluginPath = plugin.loadPath();
    MString menuPath = MString("python(\"exec(open(\\\"") + pluginPath + MString("/nexusMenuPy.py\\\").read())\");");
    sprintf_s(buffer, 2038, menuPath.asChar(), pluginPath);
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

    status = plugin.deregisterNode(NexusRigidBodyNode::id);
    if (!status)
    {
        status.perror("deregisterNode NexusRigidBodyNode");
        return status;
    }

    return status;
}