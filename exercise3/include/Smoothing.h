// This source code is property of the Computer Graphics and Visualization 
// chair of the TU Dresden. Do not distribute! 
// Copyright (C) CGV TU Dresden - All Rights Reserved

#pragma once

#include "util/OpenMeshUtils.h"

//Updates the vertex positions by Laplacian smoothing
void SmoothUniformLaplacian(HEMesh& m, float lamda, unsigned int iterations, OpenMesh::VPropHandleT<OpenMesh::Vec3f> vertexCogProperty);
//Updates the vertex positions using cotangent discretization of the Laplacian
void SmoothCotangentLaplacian(HEMesh& m, float lamda, unsigned int iterations, OpenMesh::VPropHandleT<OpenMesh::Vec3f> vertexCogProperty, OpenMesh::EPropHandleT<double> eWeights, OpenMesh::EPropHandleT<double> atheta, OpenMesh::EPropHandleT<double> btheta);
void ComputeCOG(HEMesh& m, OpenMesh::VPropHandleT<OpenMesh::Vec3f> vertexCogProperty, OpenMesh::VertexHandle vertexHandle);
void ComputeCOGCotangent(HEMesh &m,  OpenMesh::VPropHandleT<OpenMesh::Vec3f> vertexCogProperty, OpenMesh::EPropHandleT<double> eWeights, OpenMesh::EPropHandleT<double> atheta, OpenMesh::EPropHandleT<double> btheta, OpenMesh::VertexHandle vertexHandle);
void CotanWeight(HEMesh& mesh, OpenMesh::EPropHandleT<double> eWeights, OpenMesh::EPropHandleT<double>atheta, OpenMesh::EPropHandleT<double>btheta);
void SetNewPosition(HEMesh& m, float lamda, OpenMesh::VPropHandleT<OpenMesh::Vec3f> vertexCogProperty, OpenMesh::VertexHandle vertexHandle);
void AddNoise(HEMesh& m);