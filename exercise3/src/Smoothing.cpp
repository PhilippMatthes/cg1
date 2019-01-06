﻿// This source code is property of the Computer Graphics and Visualization 
// chair of the TU Dresden. Do not distribute! 
// Copyright (C) CGV TU Dresden - All Rights Reserved

#include "Smoothing.h"
#include <random>
#include <iostream>


void SmoothUniformLaplacian(HEMesh& m, float lamda, unsigned int iterations, OpenMesh::VPropHandleT<OpenMesh::Vec3f > vertexCogProperty)
{
	std::cout << "_______________________________________Uniform Laplacian Smoothing_______________________________________" << std::endl;
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

void SmoothCotangentLaplacian(HEMesh& m, float lamda, unsigned int iterations, OpenMesh::VPropHandleT<OpenMesh::Vec3f> vertexCogProperty, OpenMesh::EPropHandleT<double> eWeights, OpenMesh::EPropHandleT<double>atheta, OpenMesh::EPropHandleT<double>btheta){
	std::cout << "_______________________________________Laplace Smoothing using cotaangent discretization_______________________________________" << std::endl;

    CotanWeight(m, eWeights, atheta, btheta);

    for (unsigned int i=0; i < iterations; ++i)
    {
        //First Calculate COG for all Vertices
        for (auto v_iter = m.vertices_begin(); v_iter != m.vertices_end(); ++v_iter)
            ComputeCOGCotangent(m, vertexCogProperty, eWeights, atheta, btheta, v_iter.operator*());

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

void ComputeCOGCotangent(HEMesh &m,  OpenMesh::VPropHandleT<OpenMesh::Vec3f> vertexCogProperty, OpenMesh::EPropHandleT<double> eWeights, OpenMesh::EPropHandleT<double> atheta, OpenMesh::EPropHandleT<double> btheta, OpenMesh::VertexHandle vertexHandle) {
    double valence = 0, weight=0;
    OpenMesh::HalfedgeHandle h0, h1;
    OpenMesh::Vec3f cog = OpenMesh::Vec3f( 0.0f, 0.0f, 0.0f),  p0 = OpenMesh::Vec3f( 0.0f, 0.0f, 0.0f), p1 = OpenMesh::Vec3f( 0.0f, 0.0f, 0.0f);
    for ( auto vhe_it= m.voh_iter(vertexHandle); vhe_it.is_valid(); ++vhe_it)
    {
        p0 = m.point(m.from_vertex_handle(vhe_it.operator*()));     //pi
        p1 = m.point(m.to_vertex_handle(vhe_it.operator*()));       //pj
        auto edgeHandle = m.edge_handle(vhe_it.operator*());
        weight = m.property(eWeights, edgeHandle);                  //Get alpha{i,j] + beta{i,j}
        cog += weight * (p1 - p0);
        valence += weight;
    }
    cog/=valence;
    m.property(vertexCogProperty, vertexHandle) = (cog);
}

void CotanWeight(HEMesh& mesh, OpenMesh::EPropHandleT<double> eWeights, OpenMesh::EPropHandleT<double>atheta, OpenMesh::EPropHandleT<double>btheta) {
    double weight, a, b;
    OpenMesh::HalfedgeHandle h0, h1, h2;
    OpenMesh::Vec3f p0, p1, p2, d0, d1;
    const float pi = 3.14159265359f;

    for (OpenMesh::PolyConnectivity::EdgeIter e_it = mesh.edges_begin(); e_it != mesh.edges_end(); ++e_it) {
        weight = 0.0;

        h0 = mesh.halfedge_handle(*e_it, 0);
        p0 = mesh.point(mesh.to_vertex_handle(h0));
        h1 = mesh.halfedge_handle(*e_it, 1);
        p1 = mesh.point(mesh.to_vertex_handle(h1));

        h2 = mesh.next_halfedge_handle(h0);
        p2 = mesh.point(mesh.to_vertex_handle(h2));
        d0 = (p0 - p2); d0.normalize();
        d1 = (p1 - p2); d1.normalize();
        a = acos(dot(d0, d1));
        weight += 1.0f / tan(a);

        h2 = mesh.next_halfedge_handle(h1);
        p2 = mesh.point(mesh.to_vertex_handle(h2));
        d0 = (p0 - p2); d0.normalize();
        d1 = (p1 - p2); d1.normalize();
        b = acos(dot(d0, d1));
        weight += 1.0f / tan(b);

        mesh.property(eWeights, *e_it) = weight;
        mesh.property(atheta, *e_it) = a * 180.0 / pi; //Radians to degree
        mesh.property(btheta, *e_it) = b * 180.0 / pi; //Radians to degree
    }
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


