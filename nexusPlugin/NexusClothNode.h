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
#include<maya/MFnMesh.h>
#include <maya/MFnMeshData.h>
#include <maya/MPointArray.h>
#include <maya/MMatrix.h>
#include <maya/MDagPath.h>
#include <maya/MIOStream.h>



#define McheckErr(stat,msg)			\
	if ( MS::kSuccess != stat ) {	\
		MGlobal::displayInfo(msg);  \
		return MS::kFailure;		\
	}

class NexusClothNode : public MPxNode
{
public:
	NexusClothNode();
	~NexusClothNode() override {};
	MStatus compute(const MPlug& plug, MDataBlock& data) override;

	static void* creator();
	static MStatus initialize();

	static MObject inputMesh;
	static MObject outputGeometry;
	static MObject enableSelfCollisions;
	static MObject enableGravity;
	static MObject enableCollisions;
	static MObject enableWind;
	static MObject mass;
	static MObject stretchingStiffness;
	static MObject bendingStiffness;
	static MObject inMass;
	static MObject inKStretch;
	static MObject inKbend;
	static MObject clothStruct;
	static MTypeId id;
	//static MObject inClothMeshes;
};