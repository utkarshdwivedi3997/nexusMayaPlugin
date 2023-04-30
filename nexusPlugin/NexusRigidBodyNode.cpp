#include <fstream>
#include <sstream>
#include <vector>

#include <maya/MGlobal.h>
#include "NexusRigidBodyNode.h"

void* NexusRigidBodyNode::creator()
{
	return new NexusRigidBodyNode();
}

MObject NexusRigidBodyNode::inputMesh;
MObject NexusRigidBodyNode::outputGeometry;
MObject NexusRigidBodyNode::mass;
MObject NexusRigidBodyNode::inMass;
MObject NexusRigidBodyNode::rbStruct;
MTypeId NexusRigidBodyNode::id(0x80002);

MStatus NexusRigidBodyNode::initialize()
{
	MFnNumericAttribute numericAttr;
	MFnTypedAttribute geomAttr;
	MFnCompoundAttribute cmpdAttr;

	MStatus returnStatus;

	// Create attributes (initialization)
	NexusRigidBodyNode::mass = numericAttr.create("mass", "mass",
		MFnNumericData::kDouble,
		1, // initial data
		&returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusRigidBodyNode mass attribute\n");

	NexusRigidBodyNode::inMass = numericAttr.create("inMass", "inM",
		MFnNumericData::kDouble,
		1, // initial data
		&returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusRigidBodyNode outMass attribute\n");

	NexusRigidBodyNode::inputMesh = geomAttr.create("inputMesh","inMesh", MFnData::kMesh, MObject::kNullObj, &returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusRigidBodyNode input mesh attribute\n");
	geomAttr.setHidden(true);

	NexusRigidBodyNode::outputGeometry = geomAttr.create("outputGeometry", "oGeom", MFnData::kMesh, MObject::kNullObj, &returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusRigidBodyNode output geometry attribute\n");
	geomAttr.setHidden(true);

	NexusRigidBodyNode::rbStruct = cmpdAttr.create("rbAttributes", "rbAttr", &returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusRigidBodyNode compound attribute\n");
	returnStatus = cmpdAttr.addChild(NexusRigidBodyNode::mass);
	McheckErr(returnStatus, "ERROR adding child mass attribute\n");
	returnStatus = cmpdAttr.addChild(NexusRigidBodyNode::outputGeometry);
	McheckErr(returnStatus, "ERROR adding child output geometry attribute\n");
	cmpdAttr.setHidden(true);

	// Add attributes to node
	returnStatus = addAttribute(NexusRigidBodyNode::inMass);
	McheckErr(returnStatus, "ERROR adding inMass attribute\n");

	returnStatus = addAttribute(NexusRigidBodyNode::inputMesh);
	McheckErr(returnStatus, "ERROR adding input mesh attribute\n");

	returnStatus = addAttribute(NexusRigidBodyNode::rbStruct);
	McheckErr(returnStatus, "ERROR adding rbStruct compound attribute\n");

	// Attribute affects (changing attributes should change output)
	returnStatus = attributeAffects(NexusRigidBodyNode::inMass, NexusRigidBodyNode::rbStruct);
	McheckErr(returnStatus, "ERROR in mass attributeAffects\n");

	returnStatus = attributeAffects(NexusRigidBodyNode::inputMesh, NexusRigidBodyNode::rbStruct);
	McheckErr(returnStatus, "ERROR in mass attributeAffects\n");

	return returnStatus;
}

MStatus NexusRigidBodyNode::compute(const MPlug& plug, MDataBlock& data)
{
	MStatus returnStatus;

	if (plug == rbStruct) {

		MDataHandle inMeshDataHandle = data.inputValue(inputMesh);
		McheckErr(returnStatus, "Error getting input mesh data handle\n");
		MDataHandle inMassDataHandle = data.inputValue(inMass, &returnStatus);
		McheckErr(returnStatus, "Error getting input mesh data handle\n");

		MDataHandle rbStructDataHandle = data.outputValue(rbStruct);
		McheckErr(returnStatus, "Error getting rb struct output handle\n");

		rbStructDataHandle.child(outputGeometry).copy(inMeshDataHandle);
		rbStructDataHandle.child(mass).copy(inMassDataHandle);

		data.setClean(plug);
	}
	else
		return MS::kUnknownParameter;

	return MS::kSuccess;
}