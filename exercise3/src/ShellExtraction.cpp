// This source code is property of the Computer Graphics and Visualization 
// chair of the TU Dresden. Do not distribute! 
// Copyright (C) CGV TU Dresden - All Rights Reserved

#include "ShellExtraction.h"
#include <sample_set.h>

unsigned int ExtractShells(HEMesh& m, OpenMesh::FPropHandleT<int> perFaceShellIndex)
{
	//reset the shell indices to -1 for every face
	for (auto f : m.faces())
		m.property(perFaceShellIndex, f) = -1;

	// initial shell id label to start with
	unsigned int shellId = 0;

	/*Task 2.2.3*/
	for (auto f : m.faces()){
		if (m.property(perFaceShellIndex, f) == -1){
			connectedShell(m, perFaceShellIndex, f, shellId);
			shellId++;
		}
	}

	//since the shell Id will start from 0, but will be set to shellId++ during the if statement above, the shellIDs which are used will be in the range [0, shellID-1]
	// thus giving us the number of shellID shells which are used and returned here.
	return shellId;
}

void connectedShell(HEMesh& m, OpenMesh::FPropHandleT<int> perFaceShellIndex, OpenMesh::FaceHandle seed, unsigned int shellId){

	// Initialize new Set of connected components to en empty que of FaceHandles Q ← {}
	sample_set<OpenMesh::FaceHandle> connectedSet;

	// Insert the seed face into the queue
	//B ← {s0}
	connectedSet.insert(seed);

	// While we still have connected faces, repeat
	//While Q is not empty {
	while (!connectedSet.empty()){
		//get element from queue
		//	s ← any element of Q, we just get the first, cause why not ?
		OpenMesh::FaceHandle faceHandle = connectedSet.elements[0];

		// Q ← Q − {s}
		connectedSet.remove(faceHandle);
		//Label Face with the according shell Id
		//	Ys ← ClassLabel
		if(m.property(perFaceShellIndex, faceHandle) == -1 ){
			m.property(perFaceShellIndex, faceHandle) = shellId;
			// queue all connected faces to facehandle
			// Q ← Q + {r : r ∈ c(s) and Yr = 0}
			std::vector<OpenMesh::FaceHandle> connectedFaces = getConnectedFaces(m, perFaceShellIndex, faceHandle);

			//sample set data structure guards against duplicate elements
			for (auto connectedFace : connectedFaces)
				connectedSet.insert(connectedFace);
		}
	}
}

std::vector<OpenMesh::FaceHandle> getConnectedFaces(HEMesh& m, OpenMesh::FPropHandleT<int> perFaceShellIndex, OpenMesh::FaceHandle faceHandle){

	std::vector<OpenMesh::FaceHandle> connectedFaces;

	// iterator over all halfedges of a face
	for (OpenMesh::PolyConnectivity::FaceHalfedgeIter
		 fh_iter=m.cfh_iter(faceHandle);
		 fh_iter.is_valid();
		 ++fh_iter){
		//get the opposite halfedge handle which is incident to the neighboring face
		OpenMesh::HalfedgeHandle oppositeHalfedgeHandle = m.opposite_halfedge_handle(fh_iter.operator*());
			connectedFaces.emplace_back(m.face_handle(oppositeHalfedgeHandle));
	}

	return connectedFaces;
}