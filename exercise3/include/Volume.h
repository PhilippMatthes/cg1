// This source code is property of the Computer Graphics and Visualization 
// chair of the TU Dresden. Do not distribute! 
// Copyright (C) CGV TU Dresden - All Rights Reserved

#pragma once

#include "util/OpenMeshUtils.h"

//computes the volume enclosed by by a closed triangle mesh
//by summing u the signed volume of each face f the triangle mesh
//method IS restricted to closed triangle meshes
float ComputeVolume(const HEMesh& m);

float SignedVolumeOfTriangle(std::vector<OpenMesh::Vec3f>& points);