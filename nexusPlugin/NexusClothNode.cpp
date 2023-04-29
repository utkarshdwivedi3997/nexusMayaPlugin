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
MObject NexusClothNode::mass;
MObject NexusClothNode::inMass;
MObject NexusClothNode::stretchingStiffness;
MObject NexusClothNode::inKStretch;
MObject NexusClothNode::bendingStiffness;
MObject NexusClothNode::inKbend;
MObject NexusClothNode::enableCollisions;
MObject NexusClothNode::enableSelfCollisions;
MObject NexusClothNode::enableGravity;
MObject NexusClothNode::enableWind;
MObject NexusClothNode::inputMesh;
MObject NexusClothNode::outputGeometry;
MObject NexusClothNode::clothStruct;

NexusClothNode::NexusClothNode()
{
	/*int LENGTH = 25;
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
	NexusSolverNode::getInstance()->getSolver()->precomputeConstraints();*/
}

void* NexusClothNode::creator()
{
	return new NexusClothNode();
}

MStatus NexusClothNode::initialize()
{
	MFnNumericAttribute nAttr;
	MFnTypedAttribute geomAttr;
	MFnCompoundAttribute compoundAttr;

	MStatus returnStatus;

	// Create attributes (initialization)

	NexusClothNode::inMass = nAttr.create("mass", "m", MFnNumericData::kDouble, 1.0, &returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusClothNode mass attribute\n");

	NexusClothNode::inKStretch = nAttr.create("stretchingStiffness", "inKStretch", MFnNumericData::kDouble, 1.0, &returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusClothNode inKStretch attribute\n");
	nAttr.setMin(0.0);
	nAttr.setMax(1.0);

	NexusClothNode::inKbend = nAttr.create("bendingStiffness", "inKbend", MFnNumericData::kDouble, 1.0, &returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusClothNode inKbend attribute\n");
	nAttr.setMin(0.0);
	nAttr.setMax(1.0);

	NexusClothNode::mass = nAttr.create("outMass", "outm", MFnNumericData::kDouble, 1.0, &returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusClothNode outMass attribute\n");
	nAttr.setWritable(false);

	NexusClothNode::stretchingStiffness = nAttr.create("kStretch", "kStretch", MFnNumericData::kDouble, 1.0, &returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusClothNode stretchingStiffness attribute\n");
	nAttr.setWritable(false);

	NexusClothNode::bendingStiffness = nAttr.create("kBend", "kBend", MFnNumericData::kDouble, 1.0, &returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusClothNode bendingStiffness attribute\n");
	nAttr.setWritable(false);

	NexusClothNode::enableCollisions = nAttr.create("collisions", "col", MFnNumericData::kBoolean, false, &returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusClothNode enableCollisions attribute\n");

	NexusClothNode::enableWind = nAttr.create("wind", "wnd", MFnNumericData::kBoolean, false, &returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusClothNode enableWind attribute\n");

	NexusClothNode::enableSelfCollisions = nAttr.create("selfCollisions", "sCol", MFnNumericData::kBoolean, false, &returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusClothNode enableSelfCollisions attribute\n");

	NexusClothNode::enableGravity = nAttr.create("gravity", "gvt", MFnNumericData::kBoolean, false, &returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusClothNode enableGravity attribute\n");

	NexusClothNode::inputMesh = geomAttr.create("inputMesh", "inMesh", MFnData::kMesh, MObject::kNullObj, &returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusClothNode input mesh attribute\n");
	geomAttr.setHidden(true);

	NexusClothNode::outputGeometry = geomAttr.create("outputGeometry", "oGeom", MFnData::kMesh, MObject::kNullObj, &returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusClothNode output geometry attribute\n");
	geomAttr.setHidden(true);

	NexusClothNode::clothStruct = compoundAttr.create("clothAttributes", "clAttr", &returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusClothNode Compund attribute\n");
	returnStatus = compoundAttr.addChild(NexusClothNode::mass);
	McheckErr(returnStatus, "ERROR adding child attribute\n");
	returnStatus = compoundAttr.addChild(NexusClothNode::stretchingStiffness);
	McheckErr(returnStatus, "ERROR adding child attribute\n");
	returnStatus = compoundAttr.addChild(NexusClothNode::bendingStiffness);
	McheckErr(returnStatus, "ERROR adding child attribute\n");
	returnStatus = compoundAttr.addChild(NexusClothNode::outputGeometry);
	McheckErr(returnStatus, "ERROR adding child attribute\n");
	compoundAttr.setHidden(true);
	
	returnStatus = addAttribute(NexusClothNode::inputMesh);
	McheckErr(returnStatus, "ERROR adding input geometry attribute\n");

	returnStatus = addAttribute(NexusClothNode::inMass);
	McheckErr(returnStatus, "ERROR adding input mass attribute\n");

	returnStatus = addAttribute(NexusClothNode::inKbend);
	McheckErr(returnStatus, "ERROR adding inKBend attribute\n");

	returnStatus = addAttribute(NexusClothNode::inKStretch);
	McheckErr(returnStatus, "ERROR addinginKStretch attribute\n");

	returnStatus = addAttribute(NexusClothNode::clothStruct);
	McheckErr(returnStatus, "ERROR adding compund cloth struct attribute\n");

	// Attribute affects (changing attributes should change output)
	returnStatus = attributeAffects(NexusClothNode::inputMesh,
		NexusClothNode::clothStruct);
	McheckErr(returnStatus, "ERROR in inputMesh attributeAffects 1\n");
	returnStatus = attributeAffects(NexusClothNode::inMass,
		NexusClothNode::clothStruct);
	McheckErr(returnStatus, "ERROR in inputMesh attributeAffects 2\n");
	returnStatus = attributeAffects(NexusClothNode::inKbend,
		NexusClothNode::clothStruct);
	McheckErr(returnStatus, "ERROR in inputMesh attributeAffects 3\n");
	returnStatus = attributeAffects(NexusClothNode::inKStretch,
		NexusClothNode::clothStruct);
	McheckErr(returnStatus, "ERROR in inputMesh attributeAffects 4\n");
	/*returnStatus = attributeAffects(NexusClothNode::inMass,
		inClothMeshes);
	McheckErr(returnStatus, "ERROR in inputMesh attributeAffects 5\n");*/

	return returnStatus;
}


MStatus NexusClothNode::compute(const MPlug& plug, MDataBlock& data)
{
	MStatus returnStatus = MStatus::kSuccess;
	std::string dialogTxt = "print \"cloth size:\";";// +std::to_string(solver->getObjects().size()) + ";";
	MString dialog = dialogTxt.c_str();
	returnStatus = MGlobal::executeCommand(dialog);

	//return status;
	if (plug == clothStruct) {

		//return status;

		// Input handles

		MDataHandle inputMeshDataHandle = data.inputValue(inputMesh);

		// Output handle
		MDataHandle clothStructHandle = data.outputValue(clothStruct, &returnStatus);
		McheckErr(returnStatus, "Error getting cloth struct output handle\n");

		//clothStructHandle.child(outputGeometry).setMObject(inMesh);
		clothStructHandle.child(outputGeometry).copy(inputMeshDataHandle);
		//MFnMesh meshFn(clothStructHandle.child(outputGeometry).asMesh());

		MDataHandle inputMassHandle = data.inputValue(inMass, &returnStatus);
		McheckErr(returnStatus, "Error getting inMass handle\n");
		clothStructHandle.child(mass).copy(inputMassHandle);			
			
			//MFnMesh meshFn(inputMesh);
			/*MStringArray edgeVertices;
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
			}*/
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
		

		/*MDataHandle massHandle = data.inputValue(mass, &returnStatus);
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

		// Mesh manipulation
		MFnMeshData dataCreator;
		MObject newOutputData = dataCreator.create(&returnStatus);
		McheckErr(returnStatus, "ERROR creating outputData");

		// Read input grammar from text file

		// Sets output geometry data to newly processed data
		outputGeometryHandle.set(newOutputData);*/
		data.setClean(plug);
	}
	else
		return MS::kUnknownParameter;

	return MS::kSuccess;
}