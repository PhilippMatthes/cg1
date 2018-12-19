// This source code is property of the Computer Graphics and Visualization 
// chair of the TU Dresden. Do not distribute! 
// Copyright (C) CGV TU Dresden - All Rights Reserved

#include "Stripification.h"

#include <random>
#include "sample_set.h"




unsigned int ExtractTriStrips(HEMesh& mesh, OpenMesh::FPropHandleT<int> perFaceStripIdProperty, unsigned int nTrials)
{
	//prepare random engine
	std::mt19937 eng;

	/*
	 * The basic algorithm consists of several steps:
	 1. If there are no more triangles in the triangulation then exit.
	 2. Find the triangle t with the least number of neighbors (if more than one exists, choose arbitrary).
	 3. Start a new strip.
	 4. Insert the triangle t to the strip and remove it from the triangulation.
	 5. If there is no neighboring triangle to triangle t then go to 1.
	 6. Choose a new triangle t', neighboring to triangle t, with the least number of neighbors. If there is more than one triangle t' with the same least number of neighbors, look one level ahead. If there is a tie again, choose t' arbitrarily.
	 7. t <- t' . Go to 4.
	 */

	//initialize strip index to -1 for each face
	for (auto f : mesh.faces())
		mesh.property(perFaceStripIdProperty, f) = -1;

	int nStrips = 0;

	/*Task 2.2.5*/
	
	return nStrips;
}