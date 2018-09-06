// This source code is property of the Computer Graphics and Visualization 
// chair of the TU Dresden. Do not distribute! 
// Copyright (C) CGV TU Dresden - All Rights Reserved

#pragma once
#include <util/OpenMeshUtils.h>
#include <vector>
#include <Eigen/Core>

//a point to point correspondence 
// the first point is on mesh_A the second in mesh_B
typedef std::pair<Eigen::Vector3f, Eigen::Vector3f> correspondence;

//compute a rigid transformation (R,t) that maps the correspondences 
//to each other in a least-squares sense (map second entry to first entry)
Eigen::Affine3f CalculateRigidRegistration(std::vector<correspondence>& correspondences); 


