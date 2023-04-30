#pragma once

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

#include <maya/MIOStream.h>

#define McheckErr(stat,msg)			\
	if ( MS::kSuccess != stat ) {	\
		cerr << msg;				\
		return MS::kFailure;		\
	}

class NexusRigidBodyNode : public MPxNode {
public:
	NexusRigidBodyNode() {};
	~NexusRigidBodyNode() override {};
	static  void* creator();
	MStatus compute(const MPlug& plug, MDataBlock& data) override;
	static  MStatus initialize();

	static MObject inputMesh;
	static MObject outputGeometry;
	static MObject mass;
	static MObject inMass;
	static MObject rbStruct;
	static MTypeId id;
};