// This source code is property of the Computer Graphics and Visualization 
// chair of the TU Dresden. Do not distribute! 
// Copyright (C) CGV TU Dresden - All Rights Reserved

#include "Smoothing.h"

#include <random>
#include <Smoothing.h>
#include <iostream>


void SmoothUniformLaplacian(HEMesh& m, float lamda, unsigned int iterations, OpenMesh::VPropHandleT<OpenMesh::Vec3f > vertexCogProperty)
{
	/*Task 2.2.4*/
	//source: http://www.openflipper.org/media/Documentation/OpenFlipper-1.3/tutorial_04.html
	// smooth mesh _iterations times
	for (unsigned int i=0; i < iterations; ++i)
	{
		//First Calculate COG for all Vertices
		for (auto v_iter = m.vertices_begin(); v_iter != m.vertices_end(); ++v_iter)
			ComputeCOG(m, vertexCogProperty, v_iter.operator*());

        // Then set the new position for all vertices
		for (auto v_iter = m.vertices_begin(); v_iter != m.vertices_end(); ++v_iter)
			SetNewPosition(m, lamda, vertexCogProperty, v_iter.operator*());
	}
}

void ComputeCOG(HEMesh &m, OpenMesh::VPropHandleT<OpenMesh::Vec3f> vertexCogProperty, OpenMesh::VertexHandle vertexHandle) {
	int valence = 0;
	OpenMesh::Vec3f cog = OpenMesh::Vec3f( 0.0f, 0.0f, 0.0f);
	for ( auto vv_it= m.vv_iter(vertexHandle); vv_it; ++vv_it)
	{
        cog += m.point( vv_it );
		++valence;
	}
    m.property(vertexCogProperty, vertexHandle) = (cog/=valence);
}

void SetNewPosition(HEMesh &m, float lamda, OpenMesh::VPropHandleT<OpenMesh::Vec3f> vertexCogProperty, OpenMesh::VertexHandle vertexHandle) {
    if (m.is_boundary(vertexHandle))
        return;
    OpenMesh::Vec3f deltaPos = (m.property(vertexCogProperty, vertexHandle) - m.point(vertexHandle));
    OpenMesh::Vec3f newPosition = (m.point(vertexHandle) + lamda*deltaPos) ;
    m.set_point( vertexHandle, newPosition);
}

void AddNoise(HEMesh& m)
{
	std::mt19937 rnd;
	std::normal_distribution<float> dist;

	for (auto v : m.vertices())
	{
		OpenMesh::Vec3f n;
		m.calc_vertex_normal_correct(v, n); //normal scales with area
		float areaScale = n.norm();
		float lengthScale = sqrt(areaScale);
		n = lengthScale / areaScale * n;

		m.point(v) += 0.1f * dist(rnd) * n;
	}
}


