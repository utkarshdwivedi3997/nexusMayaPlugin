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

class NexusClothNode : public MPxNode
{
public:
	NexusClothNode() {};
	~NexusClothNode() override {};
	MStatus compute(const MPlug& plug, MDataBlock& data) override;
	static  void* creator();
	static  MStatus initialize();

	static MObject outputGeometry;
	static MObject mass;
	static MObject enableCollisions;
	static MObject enableSelfCollisions;
	static MObject enableGravity;
	static MObject enableWind;
	static MTypeId id;
};