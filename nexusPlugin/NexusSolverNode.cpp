#include "NexusSolverNode.h"

#include <maya/MFloatPointArray.h>
#include <maya/MIntArray.h>
#include <maya/MFnMesh.h>
#include <maya/MDataHandle.h>

#include <maya/MPxNode.h>
#include <maya/MPxGeometryFilter.h>
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

//Cloth-related
MObject NexusSolverNode::inClothMeshes;
MObject NexusSolverNode::inClothMesh;
MObject NexusSolverNode::inClothMass;
MObject NexusSolverNode::inClothkStretch;
MObject NexusSolverNode::inClothkBend;
MObject NexusSolverNode::outputClothMeshes;


void* NexusSolverNode::creator()
{
	return new NexusSolverNode();
}

MStatus NexusSolverNode::initialize()
{
	MFnNumericAttribute nAttr;
	MFnUnitAttribute timeStepAttr;
	MFnTypedAttribute geomAttr;
	MFnCompoundAttribute compoundAttr;

	MStatus returnStatus;

	// Create attributes (initialization)
	NexusSolverNode::gravity = nAttr.create("gravity", "g", MFnNumericData::kDouble, -9.8, &returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusSolverNode gravity attribute\n");

	NexusSolverNode::windMag = nAttr.create("windMag", "wMag", MFnNumericData::kDouble, 0.0, &returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusSolverNode windMag attribute\n");

	NexusSolverNode::windDirX = nAttr.create("windDirX", "wDirX", MFnNumericData::kDouble, 0.0, &returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusSolverNode windDirX attribute\n");	

	NexusSolverNode::windDirY = nAttr.create("windDirY", "wDirY", MFnNumericData::kDouble, 0.0, &returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusSolverNode windDirY attribute\n");

	NexusSolverNode::windDirZ = nAttr.create("windDirZ", "wDirZ", MFnNumericData::kDouble, 0.0, &returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusSolverNode windDirX attribute\n");

	NexusSolverNode::windDir = nAttr.create("windDir", "wDir", windDirX, windDirY, windDirZ, &returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusSolverNode windDir attribute\n");

	NexusSolverNode::timeStep = timeStepAttr.create("timeStep", "tSt", MFnUnitAttribute::kTime, 0.0, &returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusSolverNode timeStep attribute\n");

	NexusSolverNode::timeScale = nAttr.create("timeScale", "tSc", MFnNumericData::kDouble, 1.0, &returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusSolverNode timeScale attribute\n");

	NexusSolverNode::outputGeometry = geomAttr.create("outputGeometry", "oGeom", MFnArrayAttrsData::kDynArrayAttrs, MObject::kNullObj, &returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusSolverNode output geometry attribute\n");

	
	NexusSolverNode::inClothMass = nAttr.create("mass", "m", MFnNumericData::kDouble, 1.0, &returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusClothNode mass attribute\n");

	NexusSolverNode::inClothkStretch = nAttr.create("kStretch", "kStretch", MFnNumericData::kDouble, 1.0, &returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusClothNode inClothkStretch attribute\n");

	NexusSolverNode::inClothkBend = nAttr.create("kBend", "kBend", MFnNumericData::kDouble, 1.0, &returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusClothNode inClothkBend attribute\n");

	NexusSolverNode::inClothMesh = geomAttr.create("inClothMesh", "icm", MFnData::kMesh, MObject::kNullObj, &returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusSolverNode input cloth mesh attribute\n");

	NexusSolverNode::outputClothMeshes = geomAttr.create("outCloths", "ocls", MFnData::kMesh, MObject::kNullObj, &returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusSolverNode output cloth meshes attribute\n");
	geomAttr.setArray(true);
	geomAttr.setHidden(true);
	geomAttr.setUsesArrayDataBuilder(true);

	NexusSolverNode::inClothMeshes = compoundAttr.create("inCloths", "icls", &returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusSolverNode Input Cloths attribute\n");
	returnStatus = compoundAttr.addChild(NexusSolverNode::inClothMass);
	McheckErr(returnStatus, "ERROR adding child attribute\n");
	returnStatus = compoundAttr.addChild(NexusSolverNode::inClothkStretch);
	McheckErr(returnStatus, "ERROR adding child attribute\n");
	returnStatus = compoundAttr.addChild(NexusSolverNode::inClothkBend);
	McheckErr(returnStatus, "ERROR adding child attribute\n");
	returnStatus = compoundAttr.addChild(NexusSolverNode::inClothMesh);
	McheckErr(returnStatus, "ERROR adding child attribute\n");
	compoundAttr.setArray(true);
	//compoundAttr.setHidden(true);

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

	returnStatus = addAttribute(NexusSolverNode::inClothMeshes);
	McheckErr(returnStatus, "ERROR adding output geometry attribute\n");

	returnStatus = addAttribute(NexusSolverNode::outputClothMeshes);
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

	returnStatus = attributeAffects(NexusSolverNode::inClothMeshes,
		NexusSolverNode::outputClothMeshes);
	McheckErr(returnStatus, "ERROR in inClothMeshes attributeAffects\n");

	returnStatus = attributeAffects(NexusSolverNode::inClothMass,
		NexusSolverNode::outputClothMeshes);
	McheckErr(returnStatus, "ERROR in inClothMeshes attributeAffects\n");


	return MS::kSuccess;
}


MStatus NexusSolverNode::compute(const MPlug& plug, MDataBlock& data)

{
	MStatus returnStatus = MStatus::kSuccess;
	MGlobal::displayInfo(MString("At least solver's compute got called huh."));
	if (plug == outputClothMeshes) {
		solver = mkU<PBDSolver>();
		MGlobal::displayInfo(MString("Started from the bottom now we here"));
		MArrayDataHandle clothsInArray = data.inputArrayValue(inClothMeshes);
		//source: https://forums.autodesk.com/t5/maya-programming/working-with-arrays-c/td-p/9631440
		MArrayDataHandle outputArrayHandle = data.outputArrayValue(outputClothMeshes, &returnStatus);
		McheckErr(returnStatus, "Couldn't build the output handle for cloth meshes\n");
		MArrayDataBuilder outputArrayBuilder = outputArrayHandle.builder();

		for (uint i = 0; i < clothsInArray.elementCount(); i++) {
			clothsInArray.jumpToArrayElement(i);
			MDataHandle clothElement = clothsInArray.inputValue();
			//McheckErr(returnStatus, "Cloth array couldn't be read\n");
			float kStretch = clothElement.child(inClothkStretch).asDouble();
			float kBend = clothElement.child(inClothkBend).asDouble();
			MObject mesh = clothElement.child(inClothMesh).asMesh();
			
			outputArrayHandle.jumpToArrayElement(i);
			outputArrayHandle.outputValue().copy(clothElement.child(inClothMesh));
			
			//clothsInArray.next();
		}

		data.setClean(plug);
		return MS::kSuccess;
	}
	
	if (plug == outputGeometry) {

		MDataHandle timeStepHandle = data.inputValue(timeStep, &returnStatus);
		McheckErr(returnStatus, "Error getting timeStep data handle\n");
		MTime tStep = timeStepHandle.asTime();
		int timeVal = (int)tStep.as(MTime::kFilm);

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

		data.setClean(plug);
		return MS::kSuccess;
		
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