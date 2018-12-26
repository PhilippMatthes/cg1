// This source code is property of the Computer Graphics and Visualization 
// chair of the TU Dresden. Do not distribute! 
// Copyright (C) CGV TU Dresden - All Rights Reserved

#pragma once

#include "util/OpenMeshUtils.h"
#include "sample_set.h"

//Extracts triangle strips from the mesh, writes the Strip Id of each face in 
//perFaceStripIdProperty, and returns the number of strips.
unsigned int ExtractTriStrips(HEMesh& m, OpenMesh::FPropHandleT<int> perFaceStripIdProperty, unsigned int nTrials);
sample_set<OpenMesh::FaceHandle> calculateTriangleStrip (HEMesh& mesh,OpenMesh::FPropHandleT<int> perFaceStripIdProperty, OpenMesh::FaceHandle seed_pointer);
OpenMesh::HalfedgeHandle goForwards(HEMesh& mesh, OpenMesh::HalfedgeHandle hi, bool parity);
OpenMesh::HalfedgeHandle goBackwards(HEMesh& mesh, OpenMesh::HalfedgeHandle hi, bool parity);
