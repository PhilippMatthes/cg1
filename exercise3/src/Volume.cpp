// This source code is property of the Computer Graphics and Visualization
// chair of the TU Dresden. Do not distribute!
// Copyright (C) CGV TU Dresden - All Rights Reserved

#include "Volume.h"

#include <iostream>
#include <Volume.h>


float ComputeVolume(const HEMesh& m)
{
	float vol = 0;
	/*Task 2.2.2*/
	// iterator over all faces
	for (OpenMesh::PolyConnectivity::FaceIter f_it = m.faces_begin();
		 f_it!=m.faces_end();
		 ++f_it) {

		std::vector<OpenMesh::Vec3f> points(3);
		int i = 0;
		// iterator over all vertices of the edge in counter clockwise order, which is important
		for (OpenMesh::PolyConnectivity::FaceVertexCCWIter fv_ccwit = m.cfv_ccwbegin(*f_it);
			fv_ccwit!= m.cfv_ccwend(*f_it);
			++fv_ccwit) {
			points[i] = m.point(fv_ccwit.operator*());
			i++;
		}
		vol+= SignedVolumeOfTriangle(points);
	}
	return vol;
}

float SignedVolumeOfTriangle(std::vector<OpenMesh::Vec3f>& points) {
	return  (1.0f/6.0f) *  OpenMesh::dot(points[0], OpenMesh::cross(points[1], points[2]));
}
