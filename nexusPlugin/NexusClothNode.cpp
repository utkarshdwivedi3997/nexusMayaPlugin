#include "NexusClothNode.h"

#include <maya/MFloatPointArray.h>
#include <maya/MIntArray.h>
#include <maya/MFnMesh.h>
#include <maya/MDataHandle.h>

#include <maya/MPxNode.h>
#include <maya/MObject.h>
#include <maya/MPlug.h>
#include <maya/MFnMeshData.h>
#include <maya/MGlobal.h>

#include <maya/MIOStream.h>

MTypeId NexusClothNode::id(0x80001);
MObject NexusClothNode::inputMesh;
MObject NexusClothNode::outputGeometry;
MObject NexusClothNode::mass;
MObject NexusClothNode::enableCollisions;
MObject NexusClothNode::enableSelfCollisions;
MObject NexusClothNode::enableGravity;
MObject NexusClothNode::enableWind;

NexusClothNode::NexusClothNode() : initialized(false)
{
	int LENGTH = 25;
	int BREADTH = 15;
	uPtr<NexusCloth> nexusCloth = mkU<NexusCloth>();
	cloth = nexusCloth.get();
	cloth->setLengthAndBreadth(LENGTH, BREADTH);

	for (int i = 0; i < BREADTH; i++) {
		for (int j = 0; j < LENGTH; j++) {
			float mass = 2.0f;
			if (i == 0 && (j == 0 || j == LENGTH - 1))
			{
				mass = -1.0f;
			}
			uPtr<Particle> p = mkU<Particle>(mass);
			p->x = glm::vec3(j * 2, 20, i * 2);
			cloth->addParticle(std::move(p));
		}
	}
	NexusSolverNode::getInstance()->getSolver()->addObject(std::move(nexusCloth));
	NexusSolverNode::getInstance()->getSolver()->precomputeConstraints();
}

void* NexusClothNode::creator()
{
	return new NexusClothNode();
}

MStatus NexusClothNode::initialize()
{
	MFnNumericAttribute massAttr;
	MFnNumericAttribute enableCollisionsAttr;
	MFnNumericAttribute enableSelfCollisionsAttr;
	MFnNumericAttribute enableGravityAttr;
	MFnNumericAttribute enableWindAttr;
	MFnTypedAttribute geomAttr;

	MStatus returnStatus;

	// Create attributes (initialization)
	inputMesh = geomAttr.create("inputMesh", "im",
		MFnData::kMesh,
		MObject::kNullObj,
		&returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusClothNode input mesh attribute\n");
	geomAttr.setHidden(true);

	NexusClothNode::mass = massAttr.create("mass", "m",
		MFnNumericData::kDouble,
		1.0,
		&returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusClothNode mass attribute\n");

	NexusClothNode::enableCollisions = enableCollisionsAttr.create("collisions", "col",
		MFnNumericData::kBoolean,
		false,
		&returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusClothNode enableCollisions attribute\n");

	NexusClothNode::enableSelfCollisions = enableSelfCollisionsAttr.create("selfCollisions", "sCol",
		MFnNumericData::kBoolean,
		false,
		&returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusClothNode enableSelfCollisions attribute\n");

	NexusClothNode::enableGravity = enableGravityAttr.create("gravity", "g",
		MFnNumericData::kBoolean,
		false,
		&returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusClothNode enableGravity attribute\n");

	NexusClothNode::enableWind = enableWindAttr.create("wind", "w",
		MFnNumericData::kBoolean,
		false,
		&returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusClothNode enableWind attribute\n");

	NexusClothNode::outputGeometry = geomAttr.create("outputGeometry", "oGeom",
		MFnData::kMesh,
		MObject::kNullObj,
		&returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusClothNode output geometry attribute\n");

	// Add attributes to node
	returnStatus = addAttribute(NexusClothNode::inputMesh);
	McheckErr(returnStatus, "ERROR adding input mesh attribute\n");

	returnStatus = addAttribute(NexusClothNode::mass);
	McheckErr(returnStatus, "ERROR adding mass attribute\n");

	returnStatus = addAttribute(NexusClothNode::enableCollisions);
	McheckErr(returnStatus, "ERROR adding enableCollisions attribute\n");

	returnStatus = addAttribute(NexusClothNode::enableSelfCollisions);
	McheckErr(returnStatus, "ERROR adding enableCollisions attribute\n");

	returnStatus = addAttribute(NexusClothNode::enableGravity);
	McheckErr(returnStatus, "ERROR adding enableGravity attribute\n");

	returnStatus = addAttribute(NexusClothNode::enableWind);
	McheckErr(returnStatus, "ERROR adding enableWind attribute\n");

	returnStatus = addAttribute(NexusClothNode::outputGeometry);
	McheckErr(returnStatus, "ERROR adding output geometry attribute\n");

	// Attribute affects (changing attributes should change output)
	returnStatus = attributeAffects(NexusClothNode::inputMesh,
		NexusClothNode::outputGeometry);
	McheckErr(returnStatus, "ERROR in inputMesh attributeAffects\n");

	returnStatus = attributeAffects(NexusClothNode::mass,
		NexusClothNode::outputGeometry);
	McheckErr(returnStatus, "ERROR in mass attributeAffects\n");

	returnStatus = attributeAffects(NexusClothNode::enableCollisions,
		NexusClothNode::outputGeometry);
	McheckErr(returnStatus, "ERROR in enableCollisions attributeAffects\n");

	returnStatus = attributeAffects(NexusClothNode::enableSelfCollisions,
		NexusClothNode::outputGeometry);
	McheckErr(returnStatus, "ERROR in enableSelfCollisions attributeAffects\n");

	returnStatus = attributeAffects(NexusClothNode::enableGravity,
		NexusClothNode::outputGeometry);
	McheckErr(returnStatus, "ERROR in enableGravity attributeAffects\n");

	returnStatus = attributeAffects(NexusClothNode::enableWind,
		NexusClothNode::outputGeometry);
	McheckErr(returnStatus, "ERROR in enableWind attributeAffects\n");
	return MS::kSuccess;
}

MStatus NexusClothNode::compute(const MPlug& plug, MDataBlock& data)
{
	MStatus returnStatus = MStatus::kSuccess;
	std::string dialogTxt = "print \"cloth size:\";";// +std::to_string(solver->getObjects().size()) + ";";
	MString dialog = dialogTxt.c_str();
	MStatus status = MGlobal::executeCommand(dialog);
	//return status;
	if (plug == outputGeometry) {

		std::string dialogTxt = "print \"cloth sizess:\";";// +std::to_string(solver->getObjects().size()) + ";";
		MString dialog = dialogTxt.c_str();
		MStatus status = MGlobal::executeCommand(dialog);
		//return status;

		// Input handles
		if (!initialized) {
			MGlobal::displayInfo("My Cloth is initting hehehe: ");			
			MDataHandle inputMeshhandle = data.inputValue(inputMesh, &returnStatus);
			if (returnStatus == MS::kFailure) {
				MGlobal::displayInfo("Oopsie Daisy");
				return MS::kFailure;
			}
			McheckErr(returnStatus, "Error getting input mesh data handle\n");
			MFnMesh meshFn(inputMeshhandle.asMesh());
			//MFnMesh meshFn(inputMesh);
			MStringArray edgeVertices;
			MDagPath path;
			meshFn.getPath(path);
			
			MPointArray ptArr;
			meshFn.getPoints(ptArr, MSpace::kWorld);
			MMatrix m = meshFn.transformationMatrix(&returnStatus);
			MMatrix m2 = path.inclusiveMatrix();
			for (auto& p : ptArr) {
				p = m * p;
				MGlobal::displayInfo(MString("Pt: ") + p.x + "," + p.y + "," + p.z);
			}
			
			const float* pts = meshFn.getRawPoints(&returnStatus);
			for (int i = 0; pts[i] != '\0'; i++) {
				MGlobal::displayInfo(MString() + "i : " + i + ", pt: " + pts[i]);
			}
			/*for (int i = 0; i < meshFn.numEdges(); i++) {
				int2 verts;
				meshFn.getEdgeVertices(i, verts);
				MPoint p1, p2;
				meshFn.getPoint(verts[0], p1);
				meshFn.getPoint(verts[1], p2);				
				MGlobal::displayInfo(MString("From: ") + p1.x + "," + p1.y + "," + p1.z);
				MGlobal::displayInfo(MString("To  : ") + p2.x + "," + p2.y + "," + p2.z);
			}*/
			//initialized = true;
		}

		MDataHandle massHandle = data.inputValue(mass, &returnStatus);
		McheckErr(returnStatus, "Error getting mass data handle\n");
		double m = massHandle.asDouble();

		MDataHandle enableCollisionsHandle = data.inputValue(enableCollisions, &returnStatus);
		McheckErr(returnStatus, "Error getting enableCollisions data handle\n");
		bool col = enableCollisionsHandle.asBool();

		MDataHandle enableSelfCollisionsHandle = data.inputValue(enableSelfCollisions, &returnStatus);
		McheckErr(returnStatus, "Error getting enableSelfCollisions data handle\n");
		bool selfCol = enableSelfCollisionsHandle.asBool();

		MDataHandle enableGravityHandle = data.inputValue(enableGravity, &returnStatus);
		McheckErr(returnStatus, "Error getting enableGravity data handle\n");
		bool grav = enableGravityHandle.asBool();

		MDataHandle enableWindHandle = data.inputValue(enableWind, &returnStatus);
		McheckErr(returnStatus, "Error getting enableWind data handle\n");
		bool wind = enableWindHandle.asBool();

		// Output handle
		MDataHandle outputGeometryHandle = data.outputValue(outputGeometry, &returnStatus);
		McheckErr(returnStatus, "ERROR getting geometry data handle\n");

		// Mesh manipulation
		MFnMeshData dataCreator;
		MObject newOutputData = dataCreator.create(&returnStatus);
		McheckErr(returnStatus, "ERROR creating outputData");

		// Read input grammar from text file

		// Sets output geometry data to newly processed data
		outputGeometryHandle.set(newOutputData);
		data.setClean(plug);
	}
	else
		return MS::kUnknownParameter;

	return MS::kSuccess;
}