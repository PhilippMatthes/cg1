// This source code is property of the Computer Graphics and Visualization 
// chair of the TU Dresden. Do not distribute! 
// Copyright (C) CGV TU Dresden - All Rights Reserved

#include "Stripification.h"

#include <random>
#include <Stripification.h>

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
		for (int j = 0; j < nTrials; ++j) {
			sample_set<OpenMesh::FaceHandle> calculatedTriangleStrip = calculateTriangleStrip(seed_pointers.elements[j]);
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

sample_set<OpenMesh::FaceHandle> calculateTriangleStrip(OpenMesh::FaceHandle seed_pointer) {
	sample_set<OpenMesh::FaceHandle> triangle_strip;
	triangle_strip.insert(seed_pointer);
	return triangle_strip;
}
