// This source code is property of the Computer Graphics and Visualization 
// chair of the TU Dresden. Do not distribute! 
// Copyright (C) CGV TU Dresden - All Rights Reserved

#include "Stripification.h"

#include <random>
#include <Stripification.h>
#include <iostream>

#include "sample_set.h"


unsigned int ExtractTriStrips(HEMesh& mesh, OpenMesh::FPropHandleT<int> perFaceStripIdProperty, unsigned int nTrials)
{
	//prepare random engine
	std::mt19937 eng;

	unsigned int nStrips = 0;

	/*Task 2.2.5*/

	sample_set<OpenMesh::FaceHandle> unassigned_triangles; // Set of not yet assigned triangles (index -1)
	//initialize strip index to -1 for each face
	for (auto f : mesh.faces()){
		mesh.property(perFaceStripIdProperty, f) = -1;
		unassigned_triangles.insert(f); //store as not yet assigned triangle
	}

	int stripId = -1; //initial strip index used for triangles

	// If there are no more triangles in the triangulation then exit.
	while (!unassigned_triangles.empty()){
		//randomly select 𝑘=nTrials seed pointers inside not yet assigned triangles (index -1)
		sample_set<OpenMesh::FaceHandle> seed_pointers;
		for (int i = 0; i < nTrials; ++i) {
			seed_pointers.insert(unassigned_triangles.sample(eng));
		}

		sample_set<OpenMesh::FaceHandle> triangle_strip;

		//from each seed, determine maximum strip length stepping forward &  backward
        /*
         * IMPORTANT: loop will have seed_pointers.size() iterations, instead of nTrials, because the sample set will guard against duplicates.
         * Thus, in terms of having multiple samples return the same face, they will only be inserted into seed_pointers once.
         *  As a result the number of seed pointers can be less than nTrials.
         *  If iteration is held over nTrials instead of seed_pointers.size() this will result in an error, because invalid pointers are used for the face.
         */
		for (int j = 0; j < seed_pointers.size(); ++j) {
			sample_set<OpenMesh::FaceHandle> calculatedTriangleStrip = calculateTriangleStrip(mesh, perFaceStripIdProperty, seed_pointers.elements[j]);
			if (calculatedTriangleStrip.size() > triangle_strip.size()) // and select longest strip (greedy choice)
				triangle_strip = calculatedTriangleStrip;
		}

		//allocate new strip index
		stripId++;
		//and assign it to all triangles in the selected strip
		for (int k=0; k < triangle_strip.size(); ++k) {
			OpenMesh::FaceHandle faceHandle = triangle_strip.elements[k];
			mesh.property(perFaceStripIdProperty, faceHandle) = stripId;
			unassigned_triangles.remove(faceHandle);
		}
		nStrips++;
	}
	return nStrips;
}

sample_set<OpenMesh::FaceHandle> calculateTriangleStrip(HEMesh& mesh, OpenMesh::FPropHandleT<int> perFaceStripIdProperty, OpenMesh::FaceHandle seed_pointer)
{
    sample_set<OpenMesh::FaceHandle> triangle_strip; //triangle set which was sampled

    OpenMesh::HalfedgeHandle halfedgeHandle; //initial HalfEdge as start of triangle strip
        // iterator over all halfEdges of the triangle face
        for (OpenMesh::PolyConnectivity::FaceHalfedgeIter
                     fh_iter = mesh.cfh_iter(seed_pointer);
             fh_iter.is_valid();
             ++fh_iter) { // to make sure to select half edge with parity 0
            if (halfedgeHandle.operator<(fh_iter.operator*())) //select longest HalfEdge, this way they will always have parity 0
                halfedgeHandle = fh_iter.operator*();
        }

        triangle_strip.insert(seed_pointer); //start of triangle is the seed pointer

        bool hasNeighbors = true;    // boolean, if there are more triangles as neighbors, which can be assigned to triangle strip
        bool parity = 0; // Always start with parity 0
        bool direction = 1; // 1 = forward, 2 = backwards, could have been other type, but boolean is sufficient for this purpose

        while (hasNeighbors) {
            if (direction) {
                halfedgeHandle = goForwards(mesh, halfedgeHandle, parity); //get next HalfEdge
                parity = !parity; // if p=0 -> p=1 else p=1 -> p=0
                if (mesh.is_boundary(
                        halfedgeHandle)) // if the face is  no more connected, this is the end of the triangle strip
                {  // no more triangles in forward direction
                    direction = !direction; // change direction to go backwards
                    halfedgeHandle = mesh.fh_begin(
                            seed_pointer).operator*(); //set initial HalfEdge back to seed pointer
                } else {
                    OpenMesh::FaceHandle faceHandle = mesh.face_handle(
                            halfedgeHandle); // get the new face connected trough the HalfEdge
                    //if the face was already assigned to another triangle strip
                    if (mesh.property(perFaceStripIdProperty, faceHandle) != -1 ||
                        !(triangle_strip.index_lut.find(faceHandle) ==
                          triangle_strip.index_lut.end())) {// this is the end of the triangle strip in the forward direction
                        direction = !direction; // change direction to go backwards
                        halfedgeHandle = mesh.fh_begin(
                                seed_pointer).operator*(); //set initial HalfEdge back to seed pointer
                    } else {
                        triangle_strip.insert(faceHandle); // assign it to triangle strip
                    }
                }
            } else {
                halfedgeHandle = goBackwards(mesh, halfedgeHandle, parity); //get next HalfEdge
                parity = !parity; // if p=0 -> p=1 else p=1 -> p=0
                if (mesh.is_boundary(
                        halfedgeHandle)) // if the face is  no more connected, this is the end of the triangle strip
                    hasNeighbors = false;
                else {
                    OpenMesh::FaceHandle faceHandle = mesh.face_handle(
                            halfedgeHandle); // get the new face connected trough the HalfEdge
                    //if the face was already assigned to another triangle strip
                    if (mesh.property(perFaceStripIdProperty, faceHandle) != -1 ||
                        !(triangle_strip.index_lut.find(faceHandle) == triangle_strip.index_lut.end()))
                        hasNeighbors = false;// this is the end of the triangle strip
                    else {
                        triangle_strip.insert(faceHandle); // assign it to triangle strip
                    }
                }
            }
        }
    return triangle_strip;
}

OpenMesh::HalfedgeHandle goForwards(HEMesh& mesh, OpenMesh::HalfedgeHandle hi, bool parity) {
	if(!parity) // p = 0
		return mesh.prev_halfedge_handle(mesh.opposite_halfedge_handle(hi));
	else // p = 1
		return mesh.next_halfedge_handle(mesh.opposite_halfedge_handle(hi));
}

OpenMesh::HalfedgeHandle goBackwards(HEMesh& mesh, OpenMesh::HalfedgeHandle hi, bool parity) {
	if(!parity) // p = 0
		return mesh.opposite_halfedge_handle(mesh.prev_halfedge_handle(hi));
	else // p = 1
		return mesh.opposite_halfedge_handle(mesh.next_halfedge_handle(hi));
}


