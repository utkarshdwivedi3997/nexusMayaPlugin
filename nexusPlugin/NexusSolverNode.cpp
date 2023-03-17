#include "NexusSolverNode.h"

#include <maya/MFloatPointArray.h>
#include <maya/MIntArray.h>
#include <maya/MFnMesh.h>
#include <maya/MDataHandle.h>

#include <maya/MPxNode.h>
#include <maya/MFnCompoundAttribute.h>
#include <maya/MObject.h>
#include <maya/MPlug.h>
#include <maya/MFnMeshData.h>
#include <maya/MGlobal.h>

#include <maya/MIOStream.h>

MTypeId NexusSolverNode::id(0x80000);
MObject NexusSolverNode::forcesCmpd;
MObject NexusSolverNode::timeCmpd;
MObject NexusSolverNode::gravity;
MObject NexusSolverNode::windMag;
MObject NexusSolverNode::windDir;
MObject NexusSolverNode::windDirX;
MObject NexusSolverNode::windDirY;
MObject NexusSolverNode::windDirZ;
MObject NexusSolverNode::timeStep;
MObject NexusSolverNode::timeScale;

void* NexusSolverNode::creator()
{
	return new NexusSolverNode();
}

MStatus NexusSolverNode::initialize()
{
	MFnCompoundAttribute forcesAttr;
	MFnCompoundAttribute timeAttr;

	MFnNumericAttribute nAttr;
	MFnNumericAttribute gravAttr;
	MFnNumericAttribute windMagAttr;
	MFnNumericAttribute windDirAttr;

	MFnUnitAttribute timeStepAttr;
	MFnNumericAttribute timeScaleAttr;

	MStatus returnStatus;

	// Create attributes (initialization)
	NexusSolverNode::gravity = gravAttr.create("gravity", "g",
		MFnNumericData::kDouble,
		-9.8,
		&returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusSolverNode gravity attribute\n");

	NexusSolverNode::windMag = windMagAttr.create("windMag", "wMag",
		MFnNumericData::kDouble,
		0.0,
		&returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusSolverNode windMag attribute\n");

	NexusSolverNode::windDirX = nAttr.create("windDirX", "wDirX",
		MFnNumericData::kDouble,
		0.0,
		&returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusSolverNode windDirX attribute\n");	
	NexusSolverNode::windDirY = nAttr.create("windDirY", "wDirY",
		MFnNumericData::kDouble,
		0.0,
		&returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusSolverNode windDirY attribute\n");
	NexusSolverNode::windDirZ = nAttr.create("windDirZ", "wDirZ",
		MFnNumericData::kDouble,
		0.0,
		&returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusSolverNode windDirX attribute\n");

	NexusSolverNode::windDir = nAttr.create("windDir", "wDir",
		windDirX,
		windDirY,
		windDirZ,
		&returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusSolverNode windDir attribute\n");

	NexusSolverNode::timeStep = timeStepAttr.create("timeStep", "tSt",
		MFnUnitAttribute::kTime,
		0.0,
		&returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusSolverNode timeStep attribute\n");

	NexusSolverNode::timeScale = timeScaleAttr.create("timeScale", "tSc",
		MFnNumericData::kDouble,
		1.0,
		&returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusSolverNode timeScale attribute\n");

	// Add attributes to node
	returnStatus = addAttribute(NexusSolverNode::gravity);
	McheckErr(returnStatus, "ERROR adding windDir attribute\n");

	returnStatus = addAttribute(NexusSolverNode::windMag);
	McheckErr(returnStatus, "ERROR adding windDir attribute\n");

	returnStatus = addAttribute(NexusSolverNode::windDir);
	McheckErr(returnStatus, "ERROR adding windDir attribute\n");

	returnStatus = addAttribute(NexusSolverNode::timeStep);
	McheckErr(returnStatus, "ERROR adding timeStep attribute\n");

	returnStatus = addAttribute(NexusSolverNode::timeScale);
	McheckErr(returnStatus, "ERROR adding timeScale attribute\n");

	//// Attribute affects (changing attributes should change output)
	//returnStatus = attributeAffects(NexusSolverNode::gravity,
	//	NexusSolverNode::outputGeometry);
	//McheckErr(returnStatus, "ERROR in gravity attributeAffects\n");

	//returnStatus = attributeAffects(NexusSolverNode::windDir,
	//	NexusSolverNode::outputGeometry);
	//McheckErr(returnStatus, "ERROR in windDir attributeAffects\n");

	//returnStatus = attributeAffects(NexusSolverNode::windMag,
	//	NexusSolverNode::outputGeometry);
	//McheckErr(returnStatus, "ERROR in windMag attributeAffects\n");

	//returnStatus = attributeAffects(NexusSolverNode::timeStep,
	//	NexusSolverNode::outputGeometry);
	//McheckErr(returnStatus, "ERROR in timeSep attributeAffects\n");

	//returnStatus = attributeAffects(NexusSolverNode::timeScale,
	//	NexusSolverNode::outputGeometry);
	//McheckErr(returnStatus, "ERROR in timeScale attributeAffects\n");
	return MS::kSuccess;
}

MStatus NexusSolverNode::compute(const MPlug& plug, MDataBlock& data)

{
	MStatus returnStatus = MStatus::kSuccess;
	return returnStatus;
	if (false/*plug == outputGeometry*/) {
		// Input handles
		MDataHandle gravityHandle = data.inputValue(gravity, &returnStatus);
		McheckErr(returnStatus, "Error getting gravity data handle\n");
		double g = gravityHandle.asDouble();

		MDataHandle windDirHandle = data.inputValue(windDir, &returnStatus);
		McheckErr(returnStatus, "Error getting windDir data handle\n");
		double wd = windDirHandle.asDouble();

		MDataHandle windMagHandle = data.inputValue(windMag, &returnStatus);
		McheckErr(returnStatus, "Error getting windMag data handle\n");
		double wm = windMagHandle.asDouble();

		MDataHandle timeStepHandle = data.inputValue(timeStep, &returnStatus);
		McheckErr(returnStatus, "Error getting timeStep data handle\n");
		MTime tStep = timeStepHandle.asTime();
		int timeVal = (int)tStep.as(MTime::kFilm);

		MDataHandle timeScaleHandle = data.inputValue(timeScale, &returnStatus);
		McheckErr(returnStatus, "Error getting windMag data handle\n");
		double tSc = timeScaleHandle.asDouble();

		// Output handle
		//MDataHandle outputGeometryHandle = data.outputValue(outputGeometry, &returnStatus);
		//McheckErr(returnStatus, "ERROR getting geometry data handle\n");

		// Mesh manipulation
		MFnMeshData dataCreator;
		MObject newOutputData = dataCreator.create(&returnStatus);
		McheckErr(returnStatus, "ERROR creating outputData");

		// Read input grammar from text file

		// Sets output geometry data to newly processed data
		//outputGeometryHandle.set(newOutputData);
		data.setClean(plug);
	}
	else
		return MS::kUnknownParameter;

	return MS::kSuccess;
}