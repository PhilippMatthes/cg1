// This source code is property of the Computer Graphics and Visualization 
// chair of the TU Dresden. Do not distribute! 
// Copyright (C) CGV TU Dresden - All Rights Reserved

#pragma once

#include <gui/AbstractViewer.h>
#include <gui/SliderHelper.h>
#include <util/OpenMeshUtils.h>

#include "Registration.h"
#include <random>

class Viewer : public nse::gui::AbstractViewer
{
public:
	Viewer();

	void drawContents();

	virtual bool resizeEvent(const Eigen::Vector2i&);

private:	

	void SetupGUI();
	void MeshUpdated();	
	void BuildCorrVBOs();

	nse::math::BoundingBox<float, 3> meshBbox, expandedBbox;

	nanogui::ComboBox* shadingBtn;
	nanogui::CheckBox* chkRenderTextureMap;
	nanogui::CheckBox* chkRenderSecondMesh;
	
	HEMesh polymesh;
	MeshRenderer renderer;	

	bool hasParametrization = false;
	Eigen::Matrix4f texMapProjectionMatrix;

	Eigen::Affine3f secondMeshTransform;

	std::mt19937 rnd;
	std::vector<correspondence> correspondences;

	size_t corrCount;
	nse::gui::GLBuffer corrPositions;
	nse::gui::GLVertexArray corrVAO;
};
