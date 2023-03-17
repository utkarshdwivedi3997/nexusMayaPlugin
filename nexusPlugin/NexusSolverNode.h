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

#define McheckErr(stat,msg)			\
	if ( MS::kSuccess != stat ) {	\
		cerr << msg;				\
		return MS::kFailure;		\
	}

class NexusSolverNode : public MPxNode
{
public:
	NexusSolverNode() {};
	~NexusSolverNode() override {};
	MStatus compute(const MPlug& plug, MDataBlock& data) override;
	static  void* creator();
	static  MStatus initialize();

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
};