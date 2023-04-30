#include "NexusClothNode.h"

#include <maya/MFloatPointArray.h>
#include <maya/MIntArray.h>
#include <maya/MFnMesh.h>
#include <maya/MDataHandle.h>

#include <maya/MPxNode.h>
#include <maya/MObject.h>
#include <maya/MPlug.h>
#include <maya/MFnMeshData.h>
#include <maya/MGlobal.h>

#include <maya/MIOStream.h>

MTypeId NexusClothNode::id(0x80001);
MObject NexusClothNode::mass;
MObject NexusClothNode::inMass;
MObject NexusClothNode::stretchingStiffness;
MObject NexusClothNode::inKStretch;
MObject NexusClothNode::bendingStiffness;
MObject NexusClothNode::inKbend;
MObject NexusClothNode::enableCollisions;
MObject NexusClothNode::enableSelfCollisions;
MObject NexusClothNode::enableGravity;
MObject NexusClothNode::enableWind;
MObject NexusClothNode::inputMesh;
MObject NexusClothNode::outputGeometry;
MObject NexusClothNode::clothStruct;

void* NexusClothNode::creator()
{
	return new NexusClothNode();
}

MStatus NexusClothNode::initialize()
{
	MFnNumericAttribute nAttr;
	MFnTypedAttribute geomAttr;
	MFnCompoundAttribute compoundAttr;

	MStatus returnStatus;

	// Create attributes (initialization)

	NexusClothNode::inMass = nAttr.create("mass", "m", MFnNumericData::kDouble, 1.0, &returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusClothNode mass attribute\n");

	NexusClothNode::inKStretch = nAttr.create("stretchingStiffness", "inKStretch", MFnNumericData::kDouble, 1.0, &returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusClothNode inKStretch attribute\n");
	nAttr.setMin(0.0);
	nAttr.setMax(1.0);

	NexusClothNode::inKbend = nAttr.create("bendingStiffness", "inKbend", MFnNumericData::kDouble, 1.0, &returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusClothNode inKbend attribute\n");
	nAttr.setMin(0.0);
	nAttr.setMax(1.0);

	NexusClothNode::mass = nAttr.create("outMass", "outm", MFnNumericData::kDouble, 1.0, &returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusClothNode outMass attribute\n");
	nAttr.setWritable(false);

	NexusClothNode::stretchingStiffness = nAttr.create("kStretch", "kStretch", MFnNumericData::kDouble, 1.0, &returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusClothNode stretchingStiffness attribute\n");
	nAttr.setWritable(false);

	NexusClothNode::bendingStiffness = nAttr.create("kBend", "kBend", MFnNumericData::kDouble, 1.0, &returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusClothNode bendingStiffness attribute\n");
	nAttr.setWritable(false);

	NexusClothNode::enableCollisions = nAttr.create("collisions", "col", MFnNumericData::kBoolean, false, &returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusClothNode enableCollisions attribute\n");

	NexusClothNode::enableWind = nAttr.create("wind", "wnd", MFnNumericData::kBoolean, false, &returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusClothNode enableWind attribute\n");

	NexusClothNode::enableSelfCollisions = nAttr.create("selfCollisions", "sCol", MFnNumericData::kBoolean, false, &returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusClothNode enableSelfCollisions attribute\n");

	NexusClothNode::enableGravity = nAttr.create("gravity", "gvt", MFnNumericData::kBoolean, false, &returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusClothNode enableGravity attribute\n");

	NexusClothNode::inputMesh = geomAttr.create("inputMesh", "inMesh", MFnData::kMesh, MObject::kNullObj, &returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusClothNode input mesh attribute\n");
	geomAttr.setHidden(true);

	NexusClothNode::outputGeometry = geomAttr.create("outputGeometry", "oGeom", MFnData::kMesh, MObject::kNullObj, &returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusClothNode output geometry attribute\n");
	geomAttr.setHidden(true);

	NexusClothNode::clothStruct = compoundAttr.create("clothAttributes", "clAttr", &returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusClothNode Compund attribute\n");
	returnStatus = compoundAttr.addChild(NexusClothNode::mass);
	McheckErr(returnStatus, "ERROR adding child attribute\n");
	returnStatus = compoundAttr.addChild(NexusClothNode::stretchingStiffness);
	McheckErr(returnStatus, "ERROR adding child attribute\n");
	returnStatus = compoundAttr.addChild(NexusClothNode::bendingStiffness);
	McheckErr(returnStatus, "ERROR adding child attribute\n");
	returnStatus = compoundAttr.addChild(NexusClothNode::outputGeometry);
	McheckErr(returnStatus, "ERROR adding child attribute\n");
	compoundAttr.setHidden(true);
	
	returnStatus = addAttribute(NexusClothNode::inputMesh);
	McheckErr(returnStatus, "ERROR adding input geometry attribute\n");

	returnStatus = addAttribute(NexusClothNode::inMass);
	McheckErr(returnStatus, "ERROR adding input mass attribute\n");

	returnStatus = addAttribute(NexusClothNode::inKbend);
	McheckErr(returnStatus, "ERROR adding inKBend attribute\n");

	returnStatus = addAttribute(NexusClothNode::inKStretch);
	McheckErr(returnStatus, "ERROR addinginKStretch attribute\n");

	returnStatus = addAttribute(NexusClothNode::clothStruct);
	McheckErr(returnStatus, "ERROR adding compund cloth struct attribute\n");

	// Attribute affects (changing attributes should change output)
	returnStatus = attributeAffects(NexusClothNode::inputMesh,
		NexusClothNode::clothStruct);
	McheckErr(returnStatus, "ERROR in inputMesh attributeAffects 1\n");
	returnStatus = attributeAffects(NexusClothNode::inMass,
		NexusClothNode::clothStruct);
	McheckErr(returnStatus, "ERROR in inputMesh attributeAffects 2\n");
	returnStatus = attributeAffects(NexusClothNode::inKbend,
		NexusClothNode::clothStruct);
	McheckErr(returnStatus, "ERROR in inputMesh attributeAffects 3\n");
	returnStatus = attributeAffects(NexusClothNode::inKStretch,
		NexusClothNode::clothStruct);
	McheckErr(returnStatus, "ERROR in inputMesh attributeAffects 4\n");

	return returnStatus;
}


MStatus NexusClothNode::compute(const MPlug& plug, MDataBlock& data)
{
	MStatus returnStatus = MStatus::kSuccess;
	
	if (plug == clothStruct) {

		//copy input data to cloth's output as-is

		MDataHandle inputMeshDataHandle = data.inputValue(inputMesh);

		MDataHandle clothStructHandle = data.outputValue(clothStruct, &returnStatus);
		McheckErr(returnStatus, "Error getting cloth struct output handle\n");

		clothStructHandle.child(outputGeometry).copy(inputMeshDataHandle);

		MDataHandle inputMassHandle = data.inputValue(inMass, &returnStatus);
		McheckErr(returnStatus, "Error getting inMass handle\n");
		clothStructHandle.child(mass).copy(inputMassHandle);		

		MDataHandle inputKStretchHandle = data.inputValue(inKStretch, &returnStatus);
		McheckErr(returnStatus, "Error getting inKStretch handle\n");
		clothStructHandle.child(stretchingStiffness).copy(inputKStretchHandle);

		MDataHandle inputKBendHandle = data.inputValue(inKbend, &returnStatus);
		McheckErr(returnStatus, "Error getting inKbend handle\n");
		clothStructHandle.child(bendingStiffness).copy(inputKBendHandle);

		data.setClean(plug);
	}
	else
		return MS::kUnknownParameter;

	return MS::kSuccess;
}