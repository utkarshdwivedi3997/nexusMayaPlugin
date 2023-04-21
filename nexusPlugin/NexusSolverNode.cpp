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
MObject NexusSolverNode::outputGeometry;

void* NexusSolverNode::creator()
{
	return (void*)getInstance();
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
	MFnTypedAttribute geomAttr;

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

	NexusSolverNode::outputGeometry = geomAttr.create("outputGeometry", "oGeom",
		MFnArrayAttrsData::kDynArrayAttrs,
		MObject::kNullObj,
		&returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusSolverNode output geometry attribute\n");

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

	returnStatus = addAttribute(NexusSolverNode::outputGeometry);
	McheckErr(returnStatus, "ERROR adding output geometry attribute\n");

	//// Attribute affects (changing attributes should change output)
	returnStatus = attributeAffects(NexusSolverNode::gravity,
		NexusSolverNode::outputGeometry);
	McheckErr(returnStatus, "ERROR in gravity attributeAffects\n");

	returnStatus = attributeAffects(NexusSolverNode::windDir,
		NexusSolverNode::outputGeometry);
	McheckErr(returnStatus, "ERROR in windDir attributeAffects\n");

	returnStatus = attributeAffects(NexusSolverNode::windMag,
		NexusSolverNode::outputGeometry);
	McheckErr(returnStatus, "ERROR in windMag attributeAffects\n");

	returnStatus = attributeAffects(NexusSolverNode::timeStep,
		NexusSolverNode::outputGeometry);
	McheckErr(returnStatus, "ERROR in timeSep attributeAffects\n");

	returnStatus = attributeAffects(NexusSolverNode::timeScale,
		NexusSolverNode::outputGeometry);
	McheckErr(returnStatus, "ERROR in timeScale attributeAffects\n");
	return MS::kSuccess;
}

MStatus NexusSolverNode::compute(const MPlug& plug, MDataBlock& data)

{
	MStatus returnStatus = MStatus::kSuccess;
	//std::string dialogTxt = "print " +std::to_string(solver->getObjects().size()) + ";";
	//MString dialog = dialogTxt.c_str();
	//MGlobal::executeCommand(dialog);
	//return returnStatus;
	if (plug == outputGeometry) {
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
		MDataHandle outputGeometryHandle = data.outputValue(outputGeometry, &returnStatus);
		McheckErr(returnStatus, "ERROR getting geometry data handle\n");		
		MFnArrayAttrsData outputArrayData;
		MVectorArray outputArray;
		
		MObject outputArrayObj = outputArrayData.create(&returnStatus);
		McheckErr(returnStatus, "ERROR setting up the output array for positions from the solver.\n");
		
		MTime currentTime = MAnimControl::currentTime();
		double deltaT = (currentTime - m_lastTime).as(MTime::Unit::kSeconds);
		double frameRate = 1.f / deltaT;
		MGlobal::displayInfo("My Beautiful Frame Rate: ");
		MGlobal::displayInfo(MString() + frameRate);	
		if (deltaT == 0.f) {
			return returnStatus;
		}

		//referred the following for frame rate calculation: https://forums.autodesk.com/t5/maya-programming/c-get-frame-rate/td-p/7832521
		double playbackSpeed = (int) MAnimControl::playbackSpeed();		
		MTime fRate = MAnimControl::playbackSpeed();
		double frameRate1 = (int)fRate.as(MTime::uiUnit());

		//float deltaT = 0.0166f;//(float)(currentTime - m_lastTime).as(MTime::kFilm);
		
		//MGlobal::displayInfo("My Beautiful Frame Rate: ");
		//MGlobal::displayInfo(MString() + frameRate);			

		/*m = "print \" MUL:\";";
		dialog = (m).c_str();
		MGlobal::executeCommand(dialog);
		dialog = ("print " + std::to_string(playbackSpeed*frameRate) + ";").c_str();
		MGlobal::executeCommand(dialog);*/
		
		solver->update(deltaT);
		for (auto& obj : solver->getObjects()) {
			for (auto& p : obj->getParticles()) {
				outputArray.append(MVector(p->x.x, p->x.y, p->x.z));
			}
		}
		m_lastTime = currentTime;
		outputArrayData.vectorArray("position") = outputArray;
		outputGeometryHandle.set(outputArrayData.object());

		data.setClean(plug);
	}
	else
		return MS::kUnknownParameter;

	return MS::kSuccess;
}