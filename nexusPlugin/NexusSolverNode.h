#pragma once

#include <maya/MFnUnitAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFloatPointArray.h>
#include <maya/MIntArray.h>
#include <maya/MFnMesh.h>
#include <maya/MPxNode.h>
#include <maya/MObject.h>
#include <maya/MPlug.h>
#include <maya/MFnMeshData.h>
#include <maya/MIOStream.h>
#include <nexus/PBDSolver.h>
#include <maya/MFnVectorArrayData.h>
#include<maya/MFnArrayAttrsData.h>
#include<maya/MAnimControl.h>

#define McheckErr(stat,msg)			\
	if ( MS::kSuccess != stat ) {	\
		cerr << msg;				\
		return MS::kFailure;		\
	}

class NexusSolverNode : public MPxNode
{
private:
	uPtr<PBDSolver> solver;
	MTime m_lastTime;
	//NexusSolverNode() :solver(mkU<PBDSolver>()){};
	NexusSolverNode() :solver(mkU<PBDSolver>()), m_lastTime(0.f) {};
	NexusSolverNode(const NexusSolverNode&) = delete;
	NexusSolverNode& operator=(const NexusSolverNode&) = delete;
	~NexusSolverNode() override {};	

public:
	
	MStatus compute(const MPlug& plug, MDataBlock& data) override;
	static  void* creator();
	static  MStatus initialize();

	static NexusSolverNode* getInstance() {
		static NexusSolverNode instance;
		return &instance;
	}

	PBDSolver* getSolver() const {
		return solver.get();
	}

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
};