// This source code is property of the Computer Graphics and Visualization 
// chair of the TU Dresden. Do not distribute! 
// Copyright (C) CGV TU Dresden - All Rights Reserved

#include "SurfaceArea.h"

#include <iostream>

float ComputeSurfaceArea(const HEMesh& m)
{
	float area = 0;
	/* Task 2.2.2 */
	// http://paulbourke.net/geometry/polygonmesh/
	// iterator over all faces
	for (OpenMesh::PolyConnectivity::FaceIter
				f_it=m.faces_begin();
				f_it!=m.faces_end();
				++f_it){
		std::cout << "Iterating over Face " << f_it.operator*().idx() << std::endl;

		auto normal = m.calc_face_normal(f_it.operator*());

		// iterator over all halfedges of a face
		for (OpenMesh::PolyConnectivity::FaceHalfedgeIter
					 fh_iter=m.cfh_iter(f_it.operator*());
					 fh_iter.is_valid();
					 ++fh_iter)
		{
			auto from = m.point(m.from_vertex_handle(fh_iter.operator*()));
			auto to = m.point(m.to_vertex_handle(fh_iter.operator*()));
			std::cout << "Iterating over Halfedge from: " << from << " to " << to << std::endl;
			area += OpenMesh::dot(OpenMesh::cross(from, to)/2, normal);
		}
	}

	return area;
}