// This source code is property of the Computer Graphics and Visualization 
// chair of the TU Dresden. Do not distribute! 
// Copyright (C) CGV TU Dresden - All Rights Reserved

#pragma once

#include "util/OpenMeshUtils.h"

//Updates the vertex positions by Laplacian smoothing
void SmoothUniformLaplacian(HEMesh& m, float lambda, unsigned int iterations);

void AddNoise(HEMesh& m);