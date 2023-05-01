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
#include <maya/MItMeshPolygon.h>
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


//bool meshesAreIdentical(MObject mm1, MObject mm2) {
//	MFnMesh m1(mm1);
//	MFnMesh m2(mm2);
//	if (m1.numVertices() != m2.numVertices() ||
//		m1.numEdges() != m2.numEdges() ||
//		m1.numPolygons() != m2.numPolygons())
//		return false;
//
//	MPointArray m1Pts, m2Pts;
//	m1.getPoints(m1Pts, MSpace::kWorld);
//	m2.getPoints(m2Pts, MSpace::kWorld);
//	for (int i = 0; i < m1.numVertices(); i++) {
//		if (m1Pts[i].x != m2Pts[i].x ||
//			m1Pts[i].y != m2Pts[i].y ||
//			m1Pts[i].z != m2Pts[i].z)
//			return false;
//	}
//	return true;
//}

struct InputClothStruct {
public:
	float mass;
	float kBend;
	float kStretch;
	MObject mesh;
	InputClothStruct(float m, float kB, float kS, MObject inmesh) : mass(m), kBend(kB), kStretch(kS), mesh(inmesh) {}
};

struct InputRigidBodyStruct {
	float mass;
	MObject mesh;
	InputRigidBodyStruct(float m, MObject inmesh) : mass(m), mesh(inmesh) {}
};

class NexusSolverNode : public MPxNode
{
private:
	uPtr<PBDSolver> solver;
	std::vector<NexusCloth*> nexusCloths;
	std::vector<InputClothStruct> prevClothState;
	std::vector<NexusRigidBody*> nexusRBs;
	std::vector<InputRigidBodyStruct> prevRBState;

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