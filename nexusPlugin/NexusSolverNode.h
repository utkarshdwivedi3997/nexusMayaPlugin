#pragma once

#include "NexusClothNode.h"
#include "NexusRigidBodyNode.h"
#include <maya/MFnUnitAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnCompoundAttribute.h>
#include <maya/MFloatPointArray.h>
#include <maya/MIntArray.h>
#include <maya/MFnMesh.h>
#include <maya/MPxNode.h>
#include <maya/MObject.h>
#include <maya/MPlug.h>
#include <maya/MFnMeshData.h>
#include <maya/MDagPath.h>
#include <maya/MItMeshEdge.h>
#include <maya/MIOStream.h>
#include <nexus/PBDSolver.h>
#include <nexus/NexusCloth.h>
#include <nexus/NexusRigidBody.h>
#include <maya/MFnVectorArrayData.h>
#include<maya/MFnArrayAttrsData.h>
#include<maya/MArrayDataBuilder.h>
#include<maya/MAnimControl.h>
#include<maya/MItGeometry.h>

#define McheckErr(stat,msg)			\
	if ( MS::kSuccess != stat ) {	\
		MGlobal::displayInfo(msg);  \
		return MS::kFailure;		\
	}

class NexusSolverNode : public MPxNode
{
private:
	uPtr<PBDSolver> solver;
	std::vector<NexusCloth*> nexusCloths;
	std::vector<NexusRigidBody*> nexusRBs;

public:
	NexusSolverNode() :solver(mkU<PBDSolver>()) {};
	~NexusSolverNode() override {};
	MStatus compute(const MPlug& plug, MDataBlock& data) override;
	MStatus connectionMade(const MPlug& affectedPlug, const MPlug& inputOtherPlug, bool asSrc) override;
	static  void* creator();
	static  MStatus initialize();
	//MStatus connectionMade(const MPlug& plug, const MPlug& otherPlug, bool asSrc) override;

	// Compound grouping attributes
	static MObject forcesCmpd;
	static MObject timeCmpd;

	// Actual attributes
	static MObject gravity;
	static MObject windMag;
	static MObject windDir;
	static MObject windDirX, windDirY, windDirZ;
	static MObject timeStep;
	static MObject timeScale;
	static MTypeId id;
	static MObject outputGeometry;
	static MObject outputClothMeshes;
	static MObject outputRBMeshes;

	//Cloth related attribs
	static MObject inClothStructs;
	static MObject inClothMass;
	static MObject inClothkStretch;
	static MObject inClothkBend;
	static MObject inClothMesh;

	// RB related attributes
	static MObject inRBStructs;
	static MObject inRBMass;
	static MObject inRBMesh;
};