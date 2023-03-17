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
MObject NexusClothNode::outputGeometry;
MObject NexusClothNode::mass;
MObject NexusClothNode::enableCollisions;
MObject NexusClothNode::enableSelfCollisions;
MObject NexusClothNode::enableGravity;
MObject NexusClothNode::enableWind;


void* NexusClothNode::creator()
{
	return new NexusClothNode();
}

MStatus NexusClothNode::initialize()
{
	MFnNumericAttribute massAttr;
	MFnNumericAttribute enableCollisionsAttr;
	MFnNumericAttribute enableSelfCollisionsAttr;
	MFnNumericAttribute enableGravityAttr;
	MFnNumericAttribute enableWindAttr;
	MFnTypedAttribute geomAttr;

	MStatus returnStatus;

	// Create attributes (initialization)
	NexusClothNode::mass = massAttr.create("mass", "m",
		MFnNumericData::kDouble,
		1.0,
		&returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusSolverNode mass attribute\n");

	NexusClothNode::enableCollisions = enableCollisionsAttr.create("collisions", "col",
		MFnNumericData::kBoolean,
		false,
		&returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusSolverNode enableCollisions attribute\n");

	NexusClothNode::enableSelfCollisions = enableSelfCollisionsAttr.create("selfCollisions", "sCol",
		MFnNumericData::kBoolean,
		false,
		&returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusSolverNode enableSelfCollisions attribute\n");

	NexusClothNode::enableGravity = enableGravityAttr.create("gravity", "g",
		MFnNumericData::kBoolean,
		false,
		&returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusSolverNode enableGravity attribute\n");

	NexusClothNode::enableWind = enableWindAttr.create("wind", "w",
		MFnNumericData::kBoolean,
		false,
		&returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusSolverNode enableWind attribute\n");

	NexusClothNode::outputGeometry = geomAttr.create("outputGeometry", "oGeom",
		MFnData::kMesh,
		MObject::kNullObj,
		&returnStatus);
	McheckErr(returnStatus, "ERROR creating LSystemNode output geometry attribute\n");

	// Add attributes to node
	returnStatus = addAttribute(NexusClothNode::mass);
	McheckErr(returnStatus, "ERROR adding mass attribute\n");

	returnStatus = addAttribute(NexusClothNode::enableCollisions);
	McheckErr(returnStatus, "ERROR adding enableCollisions attribute\n");

	returnStatus = addAttribute(NexusClothNode::enableSelfCollisions);
	McheckErr(returnStatus, "ERROR adding enableCollisions attribute\n");

	returnStatus = addAttribute(NexusClothNode::enableGravity);
	McheckErr(returnStatus, "ERROR adding enableGravity attribute\n");

	returnStatus = addAttribute(NexusClothNode::enableWind);
	McheckErr(returnStatus, "ERROR adding enableWind attribute\n");

	returnStatus = addAttribute(NexusClothNode::outputGeometry);
	McheckErr(returnStatus, "ERROR adding output geometry attribute\n");

	// Attribute affects (changing attributes should change output)
	returnStatus = attributeAffects(NexusClothNode::mass,
		NexusClothNode::outputGeometry);
	McheckErr(returnStatus, "ERROR in mass attributeAffects\n");

	returnStatus = attributeAffects(NexusClothNode::enableCollisions,
		NexusClothNode::outputGeometry);
	McheckErr(returnStatus, "ERROR in enableCollisions attributeAffects\n");

	returnStatus = attributeAffects(NexusClothNode::enableSelfCollisions,
		NexusClothNode::outputGeometry);
	McheckErr(returnStatus, "ERROR in enableSelfCollisions attributeAffects\n");

	returnStatus = attributeAffects(NexusClothNode::enableGravity,
		NexusClothNode::outputGeometry);
	McheckErr(returnStatus, "ERROR in enableGravity attributeAffects\n");

	returnStatus = attributeAffects(NexusClothNode::enableWind,
		NexusClothNode::outputGeometry);
	McheckErr(returnStatus, "ERROR in enableWind attributeAffects\n");
	return MS::kSuccess;
}

MStatus NexusClothNode::compute(const MPlug& plug, MDataBlock& data)

{
	MStatus returnStatus = MStatus::kSuccess;

	if (plug == outputGeometry) {
		// Input handles
		MDataHandle massHandle = data.inputValue(mass, &returnStatus);
		McheckErr(returnStatus, "Error getting mass data handle\n");
		double m = massHandle.asDouble();

		MDataHandle enableCollisionsHandle = data.inputValue(enableCollisions, &returnStatus);
		McheckErr(returnStatus, "Error getting enableCollisions data handle\n");
		bool col = enableCollisionsHandle.asBool();

		MDataHandle enableSelfCollisionsHandle = data.inputValue(enableSelfCollisions, &returnStatus);
		McheckErr(returnStatus, "Error getting enableSelfCollisions data handle\n");
		bool selfCol = enableSelfCollisionsHandle.asBool();

		MDataHandle enableGravityHandle = data.inputValue(enableGravity, &returnStatus);
		McheckErr(returnStatus, "Error getting enableGravity data handle\n");
		bool grav = enableGravityHandle.asBool();

		MDataHandle enableWindHandle = data.inputValue(enableWind, &returnStatus);
		McheckErr(returnStatus, "Error getting enableWind data handle\n");
		bool wind = enableWindHandle.asBool();

		// Output handle
		MDataHandle outputGeometryHandle = data.outputValue(outputGeometry, &returnStatus);
		McheckErr(returnStatus, "ERROR getting geometry data handle\n");

		// Mesh manipulation
		MFnMeshData dataCreator;
		MObject newOutputData = dataCreator.create(&returnStatus);
		McheckErr(returnStatus, "ERROR creating outputData");

		// Read input grammar from text file

		// Sets output geometry data to newly processed data
		outputGeometryHandle.set(newOutputData);
		data.setClean(plug);
	}
	else
		return MS::kUnknownParameter;

	return MS::kSuccess;
}