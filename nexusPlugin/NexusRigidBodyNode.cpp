#include <NexusRigidBodyNode.h>
#include <fstream>
#include <sstream>
#include <vector>

#include <maya/MGlobal.h>

void* NexusRigidBodyNode::creator()
{
	return new NexusRigidBodyNode();
}

MObject NexusRigidBodyNode::mass;
MObject NexusRigidBodyNode::outMass;
MTypeId NexusRigidBodyNode::id(0x80002);

MStatus NexusRigidBodyNode::initialize()
{
	MFnNumericAttribute numericAttr;
	MStatus returnStatus;

	// Create attributes (initialization)
	NexusRigidBodyNode::mass = numericAttr.create("mass", "mass",
		MFnNumericData::kDouble,
		1, // initial data
		&returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusRigidBodyNode mass attribute\n");

	NexusRigidBodyNode::outMass = numericAttr.create("outMass", "outm",
		MFnNumericData::kDouble,
		1, // initial data
		&returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusRigidBodyNode outMass attribute\n");

	// Add attributes to node
	returnStatus = addAttribute(NexusRigidBodyNode::mass);
	McheckErr(returnStatus, "ERROR adding mass attribute\n");

	returnStatus = addAttribute(NexusRigidBodyNode::outMass);
	McheckErr(returnStatus, "ERROR adding mass attribute\n");

	// Attribute affects (changing attributes should change output)
	returnStatus = attributeAffects(NexusRigidBodyNode::mass, NexusRigidBodyNode::outMass);
	McheckErr(returnStatus, "ERROR in mass attributeAffects\n");

	return MS::kSuccess;
}

MStatus NexusRigidBodyNode::compute(const MPlug& plug, MDataBlock& data)
{
	MStatus returnStatus;

	if (plug == outMass) {
		MGlobal::displayInfo("This rigid body is very rigid.");
		data.setClean(plug);
	}
	else
		return MS::kUnknownParameter;

	return MS::kSuccess;
}