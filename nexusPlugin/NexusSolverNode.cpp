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
#include <nexus/Helper.h>
#include <map>

// DO NOT GET RID OF THIS DEFINE AND INCLUDE FOR VOXELIZER OTHERWISE THINGS WILL BREAK!
#define VOXELIZER_IMPLEMENTATION
#include "voxelizer.h"

MTypeId NexusSolverNode::id(0x80000);
MObject NexusSolverNode::forcesCmpd;
MObject NexusSolverNode::timeCmpd;
MObject NexusSolverNode::gravity;
MObject NexusSolverNode::solverSubsteps;
MObject NexusSolverNode::solverIterations;
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

bool InputClothsAreIdentical(InputClothStruct icl1, InputClothStruct icl2) {
	if (icl1.mass != icl2.mass ||
		icl1.kBend != icl2.kBend ||
		icl1.kStretch != icl2.kStretch)
		return false;

	MFnMesh m1(icl1.mesh);
	MFnMesh m2(icl2.mesh);
	if (m1.numVertices() != m2.numVertices() ||
		m1.numEdges() != m2.numEdges() ||
		m1.numPolygons() != m2.numPolygons())
		return false;

	MPointArray m1Pts, m2Pts;
	m1.getPoints(m1Pts, MSpace::kWorld);
	m2.getPoints(m2Pts, MSpace::kWorld);
	for (int i = 0; i < m1.numVertices(); i++) {
		if (m1Pts[i].x != m2Pts[i].x ||
			m1Pts[i].y != m2Pts[i].y ||
			m1Pts[i].z != m2Pts[i].z)
			return false;
	}
	return true;
}

bool InputRigidBodiesAreIdentical(InputRigidBodyStruct irb1, InputRigidBodyStruct irb2)
{
	if (irb1.mass != irb2.mass)
		return false;

	MFnMesh m1(irb1.mesh);
	MFnMesh m2(irb2.mesh);
	if (m1.numVertices() != m2.numVertices() ||
		m1.numEdges() != m2.numEdges() ||
		m1.numPolygons() != m2.numPolygons())
		return false;

	MPointArray m1Pts, m2Pts;
	m1.getPoints(m1Pts, MSpace::kWorld);
	m2.getPoints(m2Pts, MSpace::kWorld);
	for (int i = 0; i < m1.numVertices(); i++) {
		if (m1Pts[i].x != m2Pts[i].x ||
			m1Pts[i].y != m2Pts[i].y ||
			m1Pts[i].z != m2Pts[i].z)
			return false;
	}
	return true;
}

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

	NexusSolverNode::solverSubsteps = nAttr.create("solverSubsteps", "substeps", MFnNumericData::kInt, 3, &returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusSolverNode solverSubsteps attribute\n");

	NexusSolverNode::solverIterations = nAttr.create("solverIterations", "iters", MFnNumericData::kInt, 3, &returnStatus);
	McheckErr(returnStatus, "ERROR creating NexusSolverNode solverIterations attribute\n");

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
	returnStatus = addAttribute(NexusSolverNode::solverSubsteps);
	McheckErr(returnStatus, "ERROR adding solverSubsteps attribute\n");

	returnStatus = addAttribute(NexusSolverNode::solverIterations);
	McheckErr(returnStatus, "ERROR adding solverIterations attribute\n");

	returnStatus = addAttribute(NexusSolverNode::gravity);
	McheckErr(returnStatus, "ERROR adding gravity attribute\n");

	returnStatus = addAttribute(NexusSolverNode::windMag);
	McheckErr(returnStatus, "ERROR adding windMag attribute\n");

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
	returnStatus = attributeAffects(NexusSolverNode::solverSubsteps,  NexusSolverNode::outputGeometry);
	McheckErr(returnStatus, "ERROR in gravity solverSubsteps\n");

	returnStatus = attributeAffects(NexusSolverNode::solverIterations, NexusSolverNode::outputGeometry);
	McheckErr(returnStatus, "ERROR in gravity solverIterations\n");

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

	returnStatus = attributeAffects(NexusSolverNode::solverSubsteps, NexusSolverNode::outputClothMeshes);
	McheckErr(returnStatus, "ERROR in gravity solverSubsteps\n");

	returnStatus = attributeAffects(NexusSolverNode::solverIterations, NexusSolverNode::outputClothMeshes);
	McheckErr(returnStatus, "ERROR in gravity solverIterations\n");

	returnStatus = attributeAffects(NexusSolverNode::solverSubsteps, NexusSolverNode::outputRBMeshes);
	McheckErr(returnStatus, "ERROR in gravity solverSubsteps\n");

	returnStatus = attributeAffects(NexusSolverNode::solverIterations, NexusSolverNode::outputRBMeshes);
	McheckErr(returnStatus, "ERROR in gravity solverIterations\n");

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

		prevClothState.push_back(InputClothStruct(mass, kStretch, kBend, mesh.asMesh()));

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
		int c = 0;
		for (auto& pt : ptArr) {
			float particlesMass = mass;// / inMesh.numVertices();
			glm::vec3 pos(pt.x, pt.y, pt.z);
			//uPtr<Particle> p = mkU<Particle>(pos, glm::vec3(0.f), -1, (c == 0 || c == 30) ? -1: particlesMass, FIXED_PARTICLE_SIZE);
			uPtr<Particle> p = mkU<Particle>(pos, glm::vec3(0.f), -1, particlesMass, FIXED_PARTICLE_SIZE);
			for (auto& pairs : pinnedClothVerts) {
				if (pairs.first == nexusCloths.size() - 1 && pairs.second == c) {
					currCloth->fixParticle(p.get());
				}
			}
			currCloth->addParticle(std::move(p));
			c++;
		}

		//stretching constraints
		for (unsigned int e = 0; e < inMesh.numEdges(); e++) {
			int2 vertdIds;
			inMesh.getEdgeVertices(e, vertdIds);
			Particle* p1 = currCloth->getParticles().at(vertdIds[0]).get();
			Particle* p2 = currCloth->getParticles().at(vertdIds[1]).get();
			currCloth->addStretchConstraint(p1, p2, glm::distance(p1->x, p2->x), kStretch);
		}

		//bending constraints
		MIntArray triCounts;
		MIntArray triVerts;
		std::map<std::pair<int, int>, std::vector<int>> edgesToThirdVertexOfFace;
		inMesh.getTriangles(triCounts, triVerts);
		for (UINT i = 0, offset = 0; i < triCounts.length(); i++, offset += triCounts[i]) {
			for(UINT j = 0; j < triCounts[i]; j++) {
				int index = 3 * (offset + j);
				for (int k = 0; k < 3; k++) {
					int v1 = triVerts[index + (k % 3)];
					int v2 = triVerts[index + ((k+1) % 3)];
					int v3 = triVerts[index + ((k + 2) % 3)];
					std::pair<int, int> edge = std::make_pair<>(glm::min(v1, v2), glm::max(v1, v2));
					if (edgesToThirdVertexOfFace.find(edge) == edgesToThirdVertexOfFace.end()) {
						std::vector<int> arrayOfThirdVertexIndices{ v3 };
						edgesToThirdVertexOfFace.insert(std::make_pair(edge, arrayOfThirdVertexIndices));
					}
					else {
						edgesToThirdVertexOfFace.at(edge).push_back(v3);
					}
				}
			}
		}
		for (auto& entry : edgesToThirdVertexOfFace) {
			if (entry.second.size() == 2) {
				Particle* p1 = currCloth->getParticles().at(entry.first.first).get();
				Particle* p2 = currCloth->getParticles().at(entry.first.second).get();
				Particle* p3 = currCloth->getParticles().at(entry.second[0]).get();
				Particle* p4 = currCloth->getParticles().at(entry.second[1]).get();
				currCloth->addBendingConstraint(p1, p2, p3, p4, kBend);
			}
		}

		//add to solver
		solver->addObject(std::move(currCloth));
	}
	else if (affectedPlug == inRBStructs)
	{
		//get data from the output attribute of rb, which is a struct (compound attribute)		
		int lastIndex = affectedPlug.numElements();
		MDataHandle rbHandle;
		inputOtherPlug.getValue(rbHandle);
		float mass = rbHandle.child(NexusRigidBodyNode::mass).asDouble();
		MDataHandle mesh = rbHandle.child(NexusRigidBodyNode::outputGeometry);

		//copy the incoming mesh as-is to the last element of outputRBMeshes
		MStatus returnStatus;
		MArrayDataHandle outputArrayHandle = data.outputArrayValue(outputRBMeshes, &returnStatus);
		McheckErr(returnStatus, "Couldn't build the output handle for rb meshes\n");
		MArrayDataBuilder outputArrayBuilder = outputArrayHandle.builder();
		MDataHandle newElement = outputArrayBuilder.addElement(lastIndex);
		//newElement.jumpToArrayElement(lastIndex);
		newElement.copy(mesh);

		//add this new rb to the solver
		uPtr<NexusRigidBody> currRB = mkU<NexusRigidBody>();
		nexusRBs.push_back(currRB.get());
		MPointArray ptArr;
		MIntArray cArr, vArr;
		MFnMesh inMesh = mesh.asMesh();
		inMesh.getPoints(ptArr, MSpace::kWorld);
		inMesh.getTriangles(cArr, vArr);

		std::vector<vec3> verts;
		for (int i = 0; i < ptArr.length(); i++)
		{
			verts.push_back(vec3(ptArr[i].x, ptArr[i].y, ptArr[i].z));
			//std::string s = "(" + std::to_string(ptArr[i].x) + ", " + std::to_string(ptArr[i].y) + ", " + std::to_string(ptArr[i].z) + ")";
			//MGlobal::displayInfo(MString(s.c_str()));
		}

		//MGlobal::displayInfo(MString("============= INDICES ============="));

		std::vector<int> indices;
		for (int i = 0; i < vArr.length(); i++)
		{
			indices.push_back(vArr[i]);
			//std::string s = std::to_string(vArr[i]);
			//MGlobal::displayInfo(MString(s.c_str()));
		}

		vx_point_cloud_t* voxelizedMesh = Helper::Voxelize(verts, indices, FIXED_PARTICLE_SIZE,
			FIXED_PARTICLE_SIZE,
			FIXED_PARTICLE_SIZE, 0.01f);

		float particleMass = mass;// / voxelizedMesh->nvertices;

		std::string s = "num voxelized verts: " + std::to_string(voxelizedMesh->nvertices);
		MGlobal::displayInfo(MString(s.c_str()));

		int objId = NexusRigidBody::getObjectID();
		for (int i = 0; i < voxelizedMesh->nvertices; i++)
		{
			vx_vertex_t v = voxelizedMesh->vertices[i];
			vec3 pos = vec3(v.x, v.y, v.z);
			uPtr<Particle> p = mkU<Particle>(pos, glm::vec3(0.f), objId, particleMass, FIXED_PARTICLE_SIZE);
			currRB->addParticle(std::move(p));
		}

		currRB->setOriginalVerts(verts);
		currRB->preComputeConstraints();
		solver->addObject(std::move(currRB));
	}
	return MPxNode::connectionMade(affectedPlug, inputOtherPlug, asSrc);
}
MStatus NexusSolverNode::pinCloth(int clothId, int vertId) {
	NexusCloth* cloth = nexusCloths.at(clothId);
	Particle* p = cloth->getParticles().at(vertId).get();
	cloth->fixParticle(p);
	pinnedClothVerts.push_back(std::make_pair(clothId, vertId));
	return MStatus::kSuccess;
}

MStatus NexusSolverNode::compute(const MPlug& plug, MDataBlock& data)
{
	MStatus returnStatus = MStatus::kSuccess;
	bool resetSolver = false;
	bool instancedRendering = false;
	
	if (plug == outputGeometry) {
		instancedRendering = true;
	}
	else if (plug == outputClothMeshes || plug == outputRBMeshes) {
#pragma region Check if cloth structs have changed
		//check if the input cloths have changed from the previous state or not. If they have, we reset the solver
		MArrayDataHandle clothsInArray = data.inputArrayValue(inClothStructs);
		//source: https://forums.autodesk.com/t5/maya-programming/working-with-arrays-c/td-p/9631440
		MArrayDataHandle outputArrayHandle = data.outputArrayValue(outputClothMeshes, &returnStatus);
		MArrayDataBuilder outputArrayBuilder = outputArrayHandle.builder();

		if (clothsInArray.elementCount() != prevClothState.size()) { //if now we added or deleted a mesh, we need to reset the solver state
			resetSolver = true;
		}
		for (unsigned int i = 0; i < clothsInArray.elementCount(); i++) {
			clothsInArray.jumpToArrayElement(i);
			MDataHandle clothElement = clothsInArray.inputValue();
			float kStretch = clothElement.child(inClothkStretch).asDouble();
			float kBend = clothElement.child(inClothkBend).asDouble();
			float mass = clothElement.child(inClothMass).asDouble();
			MObject mesh = clothElement.child(inClothMesh).asMesh();

			InputClothStruct currIcl(mass, kStretch, kBend, mesh);
			//if meshes were added or deleted
			if (i >= prevClothState.size()) {
				resetSolver = true;
				prevClothState.push_back(currIcl);
			}
			//check if existing meshes are identical
			else if (!InputClothsAreIdentical(currIcl, prevClothState[i])) {
				resetSolver = true;
				prevClothState[i] = currIcl;
			}
		}
#pragma endregion
#pragma region Check if rigidbody structs have changed
		MArrayDataHandle rbsInArray = data.inputArrayValue(inRBStructs);
		outputArrayHandle = data.outputArrayValue(outputRBMeshes, &returnStatus);
		outputArrayBuilder = outputArrayHandle.builder();

		if (rbsInArray.elementCount() != prevRBState.size()) { //if now we added or deleted a mesh, we need to reset the solver state
			resetSolver = true;
		}

		for (unsigned int i = 0; i < rbsInArray.elementCount(); i++) {
			rbsInArray.jumpToArrayElement(i);
			MDataHandle rbElement = rbsInArray.inputValue();
			float mass = rbElement.child(inRBMass).asDouble();
			MObject mesh = rbElement.child(inRBMesh).asMesh();

			InputRigidBodyStruct currIrb1(mass, mesh);
			//if meshes were added or deleted
			if (i >= prevRBState.size()) {
				resetSolver = true;
				prevRBState.push_back(currIrb1);
			}
			//check if existing meshes are identical
			else if (!InputRigidBodiesAreIdentical(currIrb1, prevRBState[i])) {
				resetSolver = true;
				prevRBState[i] = currIrb1;
			}
		}
#pragma endregion
	}
	else if (plug == timeStep) {
		if (MAnimControl::isPlaying()) {
			MTime mfps(1, MTime::kSeconds);
			double fps = mfps.asUnits(MTime::uiUnit());
			double deltaT = 1.f / fps;

			vec3 grav = vec3(0.0f, data.inputValue(gravity).asDouble(), 0.0f);
			//MGlobal::displayInfo(MString("gravity: ") + grav.x + MString(", ") + grav.y + MString(", ") + grav.z);
			solver->solverAttributes.gravity = grav;
			int solverIter = data.inputValue(solverIterations).asInt();
			solver->solverAttributes.solverIterations = solverIter;
			int solverSub = data.inputValue(solverSubsteps).asInt();
			solver->solverAttributes.solverSubsteps = solverSub;
			//MGlobal::displayInfo(MString("substeps: ") + solverSub);
			//MGlobal::displayInfo(MString("iterations: ") + solverIter);

			double windM = data.inputValue(windMag).asDouble();
			vec3 wind = windM * vec3(data.inputValue(windDirX).asDouble(),
				data.inputValue(windDirY).asDouble(),
				data.inputValue(windDirZ).asDouble());
			//MGlobal::displayInfo(MString("wind: ") + wind.x + MString(", ") + wind.y + MString(", ") + wind.z);
			solver->solverAttributes.wind = wind;

			solver->update(deltaT);
		}
		data.setClean(plug);
		return MStatus::kSuccess;
	}
	else {
		return MStatus::kUnknownParameter;
	}

	if (resetSolver) {
		MGlobal::displayInfo("Resetting the solver");
		solver = mkU<PBDSolver>();
		nexusCloths.clear();
		nexusRBs.clear();
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
#pragma region CLOTH
		MArrayDataHandle clothsInArray = data.inputArrayValue(inClothStructs);
		//source: https://forums.autodesk.com/t5/maya-programming/working-with-arrays-c/td-p/9631440
		MArrayDataHandle outputArrayHandle = data.outputArrayValue(outputClothMeshes, &returnStatus);
		MArrayDataBuilder outputArrayBuilder = outputArrayHandle.builder();

		for (unsigned int i = 0; i < clothsInArray.elementCount(); i++) {
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
					float particleMass = mass;// / mesh.numVertices();
					
					//if (c == 0 || c == 30) particleMass = -1;
					uPtr<Particle> p = mkU<Particle>(pos, glm::vec3(0.f), -1, particleMass, FIXED_PARTICLE_SIZE);
					for (auto& pairs : pinnedClothVerts) {
						if (pairs.first == i && pairs.second == c) {
							currCloth->fixParticle(p.get());
						}
					}
					currCloth->addParticle(std::move(p));
					outPtArr.append(pt);
					c++;
				}
				//stretching constraints
				for (unsigned int e = 0; e < mesh.numEdges(); e++) {
					int2 vertdIds;
					mesh.getEdgeVertices(e, vertdIds);
					Particle* p1 = currCloth.get()->getParticles().at(vertdIds[0]).get();
					Particle* p2 = currCloth.get()->getParticles().at(vertdIds[1]).get();
					currCloth.get()->addStretchConstraint(p1, p2, glm::distance(p1->x, p2->x), kStretch);
				}

				//bending constraints
				MIntArray triCounts;
				MIntArray triVerts;
				std::map<std::pair<int, int>, std::vector<int>> edgesToThirdVertexOfFace;
				mesh.getTriangles(triCounts, triVerts);
				for (UINT i = 0, offset = 0; i < triCounts.length(); i++, offset += triCounts[i]) {
					for (UINT j = 0; j < triCounts[i]; j++) {
						int index = 3 * (offset + j);
						for (int k = 0; k < 3; k++) {
							int v1 = triVerts[index + (k % 3)];
							int v2 = triVerts[index + ((k + 1) % 3)];
							int v3 = triVerts[index + ((k + 2) % 3)];
							std::pair<int, int> edge = std::make_pair<>(glm::min(v1, v2), glm::max(v1, v2));
							if (edgesToThirdVertexOfFace.find(edge) == edgesToThirdVertexOfFace.end()) {
								std::vector<int> arrayOfThirdVertexIndices{ v3 };
								edgesToThirdVertexOfFace.insert(std::make_pair(edge, arrayOfThirdVertexIndices));
							}
							else {
								edgesToThirdVertexOfFace.at(edge).push_back(v3);
							}
						}
					}
				}
				for (auto& entry : edgesToThirdVertexOfFace) {
					if (entry.second.size() == 2) {
						Particle* p1 = currCloth->getParticles().at(entry.first.first).get();
						Particle* p2 = currCloth->getParticles().at(entry.first.second).get();
						Particle* p3 = currCloth->getParticles().at(entry.second[0]).get();
						Particle* p4 = currCloth->getParticles().at(entry.second[1]).get();
						currCloth->addBendingConstraint(p1, p2, p3, p4, kBend);
					}
				}


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
#pragma endregion
#pragma region RIGIDBODY
		MArrayDataHandle rbsInArray = data.inputArrayValue(inRBStructs);
		outputArrayHandle = data.outputArrayValue(outputRBMeshes, &returnStatus);
		outputArrayBuilder = outputArrayHandle.builder();

		for (unsigned int i = 0; i < rbsInArray.elementCount(); i++)
		{
			rbsInArray.jumpToArrayElement(i);
			MDataHandle rbElement = rbsInArray.inputValue();
			float mass = rbElement.child(inRBMass).asDouble();
			MFnMesh mesh = rbElement.child(inRBMesh).asMesh();

			MPointArray ptArr;
			MPointArray outPtArr;
			mesh.getPoints(ptArr, MSpace::kWorld);

			if (resetSolver)
			{
				//add this new rb to the solver
				uPtr<NexusRigidBody> currRB = mkU<NexusRigidBody>();
				nexusRBs.push_back(currRB.get());
				MIntArray cArr, vArr;
				mesh.getTriangles(cArr, vArr);

				std::vector<vec3> verts;
				for (int i = 0; i < ptArr.length(); i++)
				{
					verts.push_back(vec3(ptArr[i].x, ptArr[i].y, ptArr[i].z));
					//std::string s = "(" + std::to_string(ptArr[i].x) + ", " + std::to_string(ptArr[i].y) + ", " + std::to_string(ptArr[i].z) + ")";
					//MGlobal::displayInfo(MString(s.c_str()));
				}

				//MGlobal::displayInfo(MString("============= INDICES ============="));

				std::vector<int> indices;
				for (int i = 0; i < vArr.length(); i++)
				{
					indices.push_back(vArr[i]);
					//std::string s = std::to_string(vArr[i]);
					//MGlobal::displayInfo(MString(s.c_str()));
				}

				vx_point_cloud_t* voxelizedMesh = Helper::Voxelize(verts, indices, FIXED_PARTICLE_SIZE,
																			FIXED_PARTICLE_SIZE, 
																			FIXED_PARTICLE_SIZE, 0.01f);

				float particleMass = mass;// / voxelizedMesh->nvertices;

				MGlobal::displayInfo(MString("num voxelized verts: ") + voxelizedMesh->nvertices);
				int objId = NexusRigidBody::getObjectID();

				for (int i = 0; i < voxelizedMesh->nvertices; i++)
				{
					vx_vertex_t v = voxelizedMesh->vertices[i];
					vec3 pos = vec3(v.x, v.y, v.z);
					uPtr<Particle> p = mkU<Particle>(pos, glm::vec3(0.f), objId, particleMass, FIXED_PARTICLE_SIZE);
					currRB->addParticle(std::move(p));
				}

				currRB->setOriginalVerts(verts);
				currRB->preComputeConstraints();
				solver->addObject(std::move(currRB));
			}
			
			std::vector<vec3> movedVerts = nexusRBs[i]->getMovedVertices();
			for (int j = 0; j < ptArr.length(); j++) {
				glm::vec3 newPos = movedVerts[j];
				MPoint pt(newPos.x, newPos.y, newPos.z);
				outPtArr.append(pt);
			}

			MFnMesh outMesh;
			MFnMeshData meshDataFn;
			MObject newMeshObj = meshDataFn.create();
			outMesh.copy(rbElement.child(inRBMesh).asMesh(), newMeshObj);
			outMesh.setPoints(outPtArr);
			outMesh.setObject(newMeshObj);
			outputArrayHandle.jumpToArrayElement(i);
			outputArrayHandle.outputValue().setMObject(newMeshObj);
		}
#pragma endregion
	}
	data.setClean(plug);
	return MS::kSuccess;
}