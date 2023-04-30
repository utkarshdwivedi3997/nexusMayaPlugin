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
MObject NexusSolverNode::inClothStructs;
MObject NexusSolverNode::inClothMesh;
MObject NexusSolverNode::inClothMass;
MObject NexusSolverNode::inClothkStretch;
MObject NexusSolverNode::inClothkBend;
MObject NexusSolverNode::outputClothMeshes;
MObject NexusSolverNode::outputRBMeshes;

// RB related
MObject NexusSolverNode::inRBStructs;
MObject NexusSolverNode::inRBMass;
MObject NexusSolverNode::inRBMesh;


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

	// Solver-level attributes
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

	//geometry attribute for instanced rendering
	NexusSolverNode::outputGeometry = geomAttr.create("outputGeometry", "oGeom", MFnArrayAttrsData::kDynArrayAttrs, MObject::kNullObj, &returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusSolverNode output geometry attribute\n");

	//input compound attributes for cloth connections
	NexusSolverNode::inClothMass = nAttr.create("mass", "m", MFnNumericData::kDouble, 1.0, &returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusClothNode mass attribute\n");

	NexusSolverNode::inClothkStretch = nAttr.create("kStretch", "kStretch", MFnNumericData::kDouble, 1.0, &returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusClothNode inClothkStretch attribute\n");

	NexusSolverNode::inClothkBend = nAttr.create("kBend", "kBend", MFnNumericData::kDouble, 1.0, &returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusClothNode inClothkBend attribute\n");

	NexusSolverNode::inClothMesh = geomAttr.create("inClothMesh", "icm", MFnData::kMesh, MObject::kNullObj, &returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusSolverNode input cloth mesh attribute\n");

	//output array of cloth meshes
	NexusSolverNode::outputClothMeshes = geomAttr.create("outCloths", "ocls", MFnData::kMesh, MObject::kNullObj, &returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusSolverNode output cloth meshes attribute\n");
	geomAttr.setArray(true);
	geomAttr.setHidden(true);
	geomAttr.setUsesArrayDataBuilder(true);

	NexusSolverNode::inClothStructs = compoundAttr.create("inCloths", "icls", &returnStatus);
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

	// RB attributes
	NexusSolverNode::inRBMass = nAttr.create("rbMass", "rbm", MFnNumericData::kDouble, 1.0, &returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusSolverNode rbMass attribute\n");

	NexusSolverNode::inRBMesh = geomAttr.create("inRBMesh", "irbm", MFnData::kMesh, MObject::kNullObj, &returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusSolverNode input rb mesh attribute\n");

	NexusSolverNode::inRBStructs = compoundAttr.create("inRBs", "irbs", &returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusSolverNode Input RB attribute\n");
	returnStatus = compoundAttr.addChild(NexusSolverNode::inRBMass);
	McheckErr(returnStatus, "ERROR adding child inRBMass attribute\n");
	returnStatus = compoundAttr.addChild(NexusSolverNode::inRBMesh);
	McheckErr(returnStatus, "ERROR adding child inRBMesh attribute\n");
	compoundAttr.setArray(true);

	//output array of RB meshes
	NexusSolverNode::outputRBMeshes = geomAttr.create("outRBs", "orbs", MFnData::kMesh, MObject::kNullObj, &returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusSolverNode output rb meshes attribute\n");
	geomAttr.setArray(true);
	geomAttr.setHidden(true);
	geomAttr.setUsesArrayDataBuilder(true);

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

	returnStatus = addAttribute(NexusSolverNode::inClothStructs);
	McheckErr(returnStatus, "ERROR adding output geometry attribute\n");

	returnStatus = addAttribute(NexusSolverNode::outputClothMeshes);
	McheckErr(returnStatus, "ERROR adding output geometry attribute\n");

	returnStatus = addAttribute(NexusSolverNode::inRBStructs);
	McheckErr(returnStatus, "ERROR adding input RB structs attribute\n");

	returnStatus = addAttribute(NexusSolverNode::outputRBMeshes);
	McheckErr(returnStatus, "ERROR adding output RB meshes attribute\n");

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

	returnStatus = attributeAffects(NexusSolverNode::inClothStructs,
		NexusSolverNode::outputGeometry);
	McheckErr(returnStatus, "ERROR in inClothMeshes attributeAffects\n");

	returnStatus = attributeAffects(NexusSolverNode::inRBStructs,
		NexusSolverNode::outputGeometry);
	McheckErr(returnStatus, "ERROR in inRBStructs attributeAffects\n");

	returnStatus = attributeAffects(NexusSolverNode::inClothStructs,
		NexusSolverNode::outputClothMeshes);
	McheckErr(returnStatus, "ERROR in inClothMeshes attributeAffects\n");

	returnStatus = attributeAffects(NexusSolverNode::timeStep,
		NexusSolverNode::outputClothMeshes);
	McheckErr(returnStatus, "ERROR in timeSep attributeAffects\n");

	returnStatus = attributeAffects(NexusSolverNode::inRBStructs,
		NexusSolverNode::outputRBMeshes);
	McheckErr(returnStatus, "ERROR in inRBStructs attributeAffects\n");

	returnStatus = attributeAffects(NexusSolverNode::timeStep,
		NexusSolverNode::outputRBMeshes);
	McheckErr(returnStatus, "ERROR in inRBStructs attributeAffects\n");

	return MS::kSuccess;
}

MStatus NexusSolverNode::connectionMade(const MPlug& affectedPlug, const MPlug& inputOtherPlug, bool asSrc) {
	MDataBlock data = forceCache();
	if(affectedPlug == inClothStructs) {
		//get data from the output attribute of cloth, which is a struct (compound attribute)		
		int lastIndex = affectedPlug.numElements();
		MDataHandle handleFromCloth;
		inputOtherPlug.getValue(handleFromCloth);
		float kStretch = handleFromCloth.child(NexusClothNode::stretchingStiffness).asDouble();
		float kBend = handleFromCloth.child(NexusClothNode::bendingStiffness).asDouble();
		float mass = handleFromCloth.child(NexusClothNode::mass).asDouble();
		MDataHandle mesh = handleFromCloth.child(NexusClothNode::outputGeometry);

		//copy the incoming mesh as-is to the last element of outputClothMeshes
		MStatus returnStatus;
		MArrayDataHandle outputArrayHandle = data.outputArrayValue(outputClothMeshes, &returnStatus);
		McheckErr(returnStatus, "Couldn't build the output handle for cloth meshes\n");
		MArrayDataBuilder outputArrayBuilder = outputArrayHandle.builder();
		MDataHandle newElement = outputArrayBuilder.addElement(lastIndex);
		//newElement.jumpToArrayElement(lastIndex);
		newElement.copy(mesh);

		//add this new cloth to the solver
		uPtr<NexusCloth> currCloth = mkU<NexusCloth>();
		nexusCloths.push_back(currCloth.get());
		MPointArray ptArr;
		MFnMesh inMesh = mesh.asMesh();
		inMesh.getPoints(ptArr, MSpace::kWorld);
		float particlesMass = mass / inMesh.numVertices();
		int c = 0;
		for (auto& pt : ptArr) {
			glm::vec3 pos(pt.x, pt.y, pt.z);
			uPtr<Particle> p = mkU<Particle>(pos, glm::vec3(0.f), 0, (c == 0 || c == 30) ? -1: particlesMass);
			currCloth->addParticle(std::move(p));
			c++;
		}

		//stretching constraints
		for (uint e = 0; e < inMesh.numEdges(); e++) {
			int2 vertdIds;
			inMesh.getEdgeVertices(e, vertdIds);
			Particle* p1 = currCloth.get()->getParticles().at(vertdIds[0]).get();
			Particle* p2 = currCloth.get()->getParticles().at(vertdIds[1]).get();
			currCloth.get()->addStretchConstraint(p1, p2, glm::distance(p1->x, p2->x), kStretch);
		}

		//bending constraints


		//add to solver
		solver->addObject(std::move(currCloth));
	}
	return MPxNode::connectionMade(affectedPlug, inputOtherPlug, asSrc);
}

MStatus NexusSolverNode::compute(const MPlug& plug, MDataBlock& data)
{
	MStatus returnStatus = MStatus::kSuccess;
	bool resetSolver = false;
	bool instancedRendering = false;

	if (plug == inClothStructs) {
		resetSolver = true;
	}
	
	if (plug == outputGeometry) {
		instancedRendering = true;
	}
	else if (plug == outputClothMeshes) {
		//MGlobal::displayInfo(MString("Resetting the solver."));
	}
	else if (plug == timeStep) {
		if (MAnimControl::isPlaying()) {
			MTime mfps(1, MTime::kSeconds);
			double fps = mfps.asUnits(MTime::uiUnit());
			double deltaT = 1.f / fps;
			solver->update(deltaT);
		}
		data.setClean(plug);
		return MStatus::kSuccess;
	}
	else {
		return MStatus::kUnknownParameter;
	}

	if (resetSolver) {
		solver = mkU<PBDSolver>();
		nexusCloths.clear();
	}

	if (instancedRendering) {
		//Output handle
		MDataHandle outputGeometryHandle = data.outputValue(outputGeometry, &returnStatus);
		McheckErr(returnStatus, "ERROR getting geometry data handle\n");
		MFnArrayAttrsData outputArrayData;
		MVectorArray outputArray;

		MObject outputArrayObj = outputArrayData.create(&returnStatus);
		McheckErr(returnStatus, "ERROR setting up the output array for positions from the solver.\n");

		for (auto& obj : solver->getObjects()) {
			for (auto& p : obj->getParticles()) {
				outputArray.append(MVector(p->x.x, p->x.y, p->x.z));
			}
		}
		outputArrayData.vectorArray("position") = outputArray;
		outputGeometryHandle.set(outputArrayData.object());
	}
	else
	{
		MArrayDataHandle clothsInArray = data.inputArrayValue(inClothStructs);
		//source: https://forums.autodesk.com/t5/maya-programming/working-with-arrays-c/td-p/9631440
		MArrayDataHandle outputArrayHandle = data.outputArrayValue(outputClothMeshes, &returnStatus);
		MArrayDataBuilder outputArrayBuilder = outputArrayHandle.builder();

		for (uint i = 0; i < clothsInArray.elementCount(); i++) {
			clothsInArray.jumpToArrayElement(i);
			MDataHandle clothElement = clothsInArray.inputValue();
			float kStretch = clothElement.child(inClothkStretch).asDouble();
			float kBend = clothElement.child(inClothkBend).asDouble();
			float mass = clothElement.child(inClothMass).asDouble();
			MFnMesh mesh = clothElement.child(inClothMesh).asMesh();

			MPointArray ptArr;
			MPointArray outPtArr;
			mesh.getPoints(ptArr, MSpace::kWorld);
			//MMatrix m = meshFn.transformationMatrix(&returnStatus);
			//MMatrix m2 = path.inclusiveMatrix();

			//if resetting the solver, need to set up the constraints again
			if (resetSolver) {
				int c = 0;
				uPtr<NexusCloth> currCloth = mkU<NexusCloth>();
				nexusCloths.push_back(currCloth.get());
				for (auto& pt : ptArr) {					
					glm::vec3 pos(pt.x, pt.y, pt.z);
					float particleMass = mass / mesh.numVertices();
					if (c == 0 || c == 30) particleMass = -1;
					uPtr<Particle> p = mkU<Particle>(pos, glm::vec3(0.f), 0, particleMass);
					currCloth->addParticle(std::move(p));
					outPtArr.append(pt);
					c++;
				}
				//stretching constraints
				for (uint e = 0; e < mesh.numEdges(); e++) {
					int2 vertdIds;
					mesh.getEdgeVertices(e, vertdIds);
					Particle* p1 = currCloth.get()->getParticles().at(vertdIds[0]).get();
					Particle* p2 = currCloth.get()->getParticles().at(vertdIds[1]).get();
					currCloth.get()->addStretchConstraint(p1, p2, glm::distance(p1->x, p2->x), kStretch);
				}

				//bending constraints


				//add to solver
				solver->addObject(std::move(currCloth));
			}
			//otherwise if not resetting the solver, just update the mesh vertex positions
			else {
				for (int j = 0; j < ptArr.length(); j++) {
					glm::vec3 newPos = nexusCloths[i]->getParticles().at(j).get()->x;
					MPoint pt(newPos.x, newPos.y, newPos.z);
					outPtArr.append(pt);
				}
			}

			MFnMesh outMesh;
			MFnMeshData meshDataFn;
			MObject newMeshObj = meshDataFn.create();
			outMesh.copy(clothElement.child(inClothMesh).asMesh(), newMeshObj);
			outMesh.setPoints(outPtArr);
			outMesh.setObject(newMeshObj);
			outputArrayHandle.jumpToArrayElement(i);
			outputArrayHandle.outputValue().setMObject(newMeshObj);

		}
	}
	data.setClean(plug);
	return MS::kSuccess;
}