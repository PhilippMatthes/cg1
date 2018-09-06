// This source code is property of the Computer Graphics and Visualization 
// chair of the TU Dresden. Do not distribute! 
// Copyright (C) CGV TU Dresden - All Rights Reserved

#include "Registration.h"

Eigen::Affine3f CalculateRigidRegistration(std::vector<correspondence>& correspondences)
{
	//transform to compute
	Eigen::Affine3f T = Eigen::Affine3f(Eigen::Matrix3f::Identity());

	if(correspondences.size() < 3)
		return T;	
	
	/* Task 4.2.2 */

	return T;
}
