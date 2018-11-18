// This source code is property of the Computer Graphics and Visualization 
// chair of the TU Dresden. Do not distribute! 
// Copyright (C) CGV TU Dresden - All Rights Reserved

#pragma once
#include <util/OpenMeshUtils.h>

//returns true if mesh is a topological disk
bool IsTopologicalDisk(const HEMesh& m, OpenMesh::HalfedgeHandle& outBoundary);

//different possible weights
enum WeightType { CONSTANT_WEIGHT, EDGE_LENGTH_WEIGHT, INV_EDGE_LENGTH_WEIGHT, COTAN_WEIGHT};

//compute weight w_ij for a given halfedge, where vertex i is the origin and vertex j is the target of halfedge h
template <WeightType wtype>
float Weight(HEMesh& m, OpenMesh::HalfedgeHandle h);

template<> float Weight<CONSTANT_WEIGHT>(HEMesh& m, OpenMesh::HalfedgeHandle h);
template<> float Weight<EDGE_LENGTH_WEIGHT>(HEMesh& m, OpenMesh::HalfedgeHandle h);
template<> float Weight<INV_EDGE_LENGTH_WEIGHT>(HEMesh& m, OpenMesh::HalfedgeHandle h);
template<> float Weight<COTAN_WEIGHT>(HEMesh& m, OpenMesh::HalfedgeHandle h);

//computes a parametrization (textcoords for each vertex)
//texcoords of boundary vertices are placed on a circle with center (0.5,0.5) and radius 1 intexture space
//texcoords of inner vertices are placed in the weighted mean of its one ring neighbors texcoords
//these constraints can be stacked into a sparse linear equation system which can be solved in a least squares sense
template <WeightType wtype>
bool ComputeParametrizationOfTopologicalDisk(HEMesh& mesh);