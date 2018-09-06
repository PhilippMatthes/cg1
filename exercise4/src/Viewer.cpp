// This source code is property of the Computer Graphics and Visualization 
// chair of the TU Dresden. Do not distribute! 
// Copyright (C) CGV TU Dresden - All Rights Reserved

#include "Viewer.h"

#include <nanogui/window.h>
#include <nanogui/button.h>
#include <nanogui/checkbox.h>
#include <nanogui/messagedialog.h>
#include <nanogui/layout.h>
#include <nanogui/combobox.h>

#include <OpenMesh/Core/IO/MeshIO.hh>

#include <gui/SliderHelper.h>

#include "Parametrization.h"
#include "Registration.h"

#include <gui/ShaderPool.h>

Viewer::Viewer()
	: AbstractViewer("CG1 Exercise 4"),
	renderer(polymesh),
	corrPositions(nse::gui::VertexBuffer)
{ 
	polymesh.request_vertex_texcoords2D();

	corrVAO.generate();

	SetupGUI();	
}

void Viewer::SetupGUI()
{
	auto mainWindow = SetupMainWindow();	

	auto loadFileBtn = new nanogui::Button(mainWindow, "Load Mesh");
	loadFileBtn->setCallback([this]() {
		std::vector<std::pair<std::string, std::string>> fileTypes;
		fileTypes.push_back(std::make_pair("obj", "OBJ File"));
		auto file = nanogui::file_dialog(fileTypes, false);
		if (!file.empty())
		{
			polymesh.clear();
			if (!OpenMesh::IO::read_mesh(polymesh, file))
			{
				new nanogui::MessageDialog(this, nanogui::MessageDialog::Type::Warning, "Load Mesh",
					"The specified file could not be loaded");
			}
			else
				MeshUpdated();
		}
	});		

	shadingBtn = new nanogui::ComboBox(mainWindow, { "Smooth Shading", "Flat Shading" });

	performLayout();

	auto paramWindow = new nanogui::Window(this, "Parametrization");
	paramWindow->setPosition(Eigen::Vector2i(mainWindow->position().x(), mainWindow->position().y() + mainWindow->size().y() + 15));
	paramWindow->setLayout(new nanogui::BoxLayout(nanogui::Orientation::Vertical, nanogui::Alignment::Fill, 4, 4));

	auto cmbWeightType = new nanogui::ComboBox(paramWindow, { "Constant Weight", "Edge Length Weight", "Inverse Edge Length Weight", "Cotan Weight" });

	auto parametrizeBtn = new nanogui::Button(paramWindow, "Calculate Parametrization");
	parametrizeBtn->setCallback([this, cmbWeightType]() {
		if (polymesh.n_vertices() == 0)
		{
			new nanogui::MessageDialog(this, nanogui::MessageDialog::Type::Warning, "Parametrization", "Please load a mesh for parametrization.");
		}
		else
		{
			bool success;
			switch (cmbWeightType->selectedIndex())
			{
			case 0:
				success = ComputeParametrizationOfTopologicalDisk<CONSTANT_WEIGHT>(polymesh);
				break;
			case 1:
				success = ComputeParametrizationOfTopologicalDisk<EDGE_LENGTH_WEIGHT>(polymesh);
				break;
			case 2:
				success = ComputeParametrizationOfTopologicalDisk<INV_EDGE_LENGTH_WEIGHT>(polymesh);
				break;
			case 3:
				success = ComputeParametrizationOfTopologicalDisk<COTAN_WEIGHT>(polymesh);
				break;
			}
			if (success)
			{
				renderer.Update();
				hasParametrization = true;
			}
			else
				new nanogui::MessageDialog(this, nanogui::MessageDialog::Type::Warning, "Parametrization", "Parametrization failed.");
		}
	});

	chkRenderTextureMap = new nanogui::CheckBox(paramWindow, "Render Texture Map");

	performLayout();

	auto registrationWindow = new nanogui::Window(this, "Registration");
	registrationWindow->setPosition(Eigen::Vector2i(paramWindow->position().x(), paramWindow->position().y() + paramWindow->size().y() + 15));
	registrationWindow->setLayout(new nanogui::BoxLayout(nanogui::Orientation::Vertical, nanogui::Alignment::Fill, 4, 4));

	auto rotateBtn = new nanogui::Button(registrationWindow, "Random Rotation");
	rotateBtn->setCallback([this]() {
		secondMeshTransform = Eigen::Affine3f(Eigen::Translation3f(3 * meshBbox.diagonal().x(), 0, 0));
		std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
		Eigen::Quaternionf q(dist(rnd), dist(rnd), dist(rnd), dist(rnd));
		q.normalize();
		secondMeshTransform *= q;

		correspondences.clear();
		BuildCorrVBOs();
	});

	corrCount = 10;
	nanogui::TextBox* txtCorrCount;
	auto sldCorrCount = nse::gui::AddLabeledSlider(registrationWindow, "Correspondence Count", std::make_pair(1.0f, 50.0f), (float)corrCount, txtCorrCount);
	sldCorrCount->setCallback([this, txtCorrCount](float value) {
		corrCount = (int)std::round(value);
		txtCorrCount->setValue(std::to_string(corrCount));
	});
	sldCorrCount->callback()(sldCorrCount->value());

	auto autoCorrs = new nanogui::Button(registrationWindow, "Automatic Correspondences");
	autoCorrs->setCallback([this]() {
		if (corrCount > polymesh.n_vertices())
			return;
		std::uniform_int_distribution<int> dist(0, (int)polymesh.n_vertices());

		correspondences.clear();

		for (int i = 0; i < corrCount; ++i)
		{
			auto v = polymesh.vertex_handle(dist(rnd));
			auto p = ToEigenVector(polymesh.point(v));
			correspondences.push_back(std::make_pair(p, secondMeshTransform * p));
		}

		BuildCorrVBOs();
	});

	auto distortCorrespondencesBtn = new nanogui::Button(registrationWindow, "Distort Correspondences");
	distortCorrespondencesBtn->setCallback([this]() {
		std::normal_distribution<float> dist(0.0f, meshBbox.diagonal().norm() * 0.01f);
		for (auto& c : correspondences)
			c.first += Eigen::Vector3f(dist(rnd), dist(rnd), dist(rnd));
		BuildCorrVBOs();
	});

	auto registerBtn = new nanogui::Button(registrationWindow, "Register");
	registerBtn->setCallback([this]() {
		auto T = CalculateRigidRegistration(correspondences);
		secondMeshTransform = T * secondMeshTransform;

		for (auto& c : correspondences)
			c.second = T * c.second;
		BuildCorrVBOs();
	});

	chkRenderSecondMesh = new nanogui::CheckBox(registrationWindow, "Render Second Mesh", [this](bool checked) {
		if (checked)
			camera().FocusOnBBox(expandedBbox);
		else
			camera().FocusOnBBox(meshBbox);
	});

	performLayout();

	auto maxWidth = std::max(mainWindow->size().x(), std::max(paramWindow->size().x(), registrationWindow->size().x()));
	mainWindow->setFixedWidth(maxWidth);
	paramWindow->setFixedWidth(maxWidth);
	registrationWindow->setFixedWidth(maxWidth);

	performLayout();
}

void Viewer::MeshUpdated()
{
	//calculate the bounding Box of the mesh
	meshBbox.reset();
	for (auto v : polymesh.vertices())
		meshBbox.expand(ToEigenVector(polymesh.point(v)));

	expandedBbox = meshBbox;
	secondMeshTransform = Eigen::Affine3f(Eigen::Translation3f(3 * meshBbox.diagonal().x(), 0, 0));
	expandedBbox.max.x() += 3 * meshBbox.diagonal().x();

	if(chkRenderSecondMesh->checked())
		camera().FocusOnBBox(expandedBbox);
	else
		camera().FocusOnBBox(meshBbox);

	polymesh.triangulate();	
	correspondences.clear();
	
	hasParametrization = false;

	renderer.Update();
	BuildCorrVBOs();
}

void Viewer::BuildCorrVBOs()
{
	std::vector<Eigen::Vector4f> pos;
	pos.reserve(correspondences.size() * 2);
	for (auto& c : correspondences)
	{
		pos.push_back(Eigen::Vector4f(c.first.x(), c.first.y(), c.first.z(), 1.0f));
		pos.push_back(Eigen::Vector4f(c.second.x(), c.second.y(), c.second.z(), 1.0f));
	}

	corrVAO.bind();
	ShaderPool::Instance()->simpleShader.bind();
	corrPositions.uploadData(pos).bindToAttribute("position");
	corrVAO.unbind();
}

bool Viewer::resizeEvent(const Eigen::Vector2i& size)
{
	AbstractViewer::resizeEvent(size);

	float ratio = (float)size.x() / size.y();
	Eigen::Affine3f proj = Eigen::Translation3f(1.0f - 0.1f / ratio - 1.8f / ratio, -0.9f, 0.0f) * Eigen::AlignedScaling3f(1.8f / ratio, 1.8f, 1.0f);
	texMapProjectionMatrix = proj.matrix();
	
	return true;
}

void Viewer::drawContents()
{
	glEnable(GL_DEPTH_TEST);

	if (!polymesh.vertices_empty())
	{
		Eigen::Matrix4f view, proj;
		camera().ComputeCameraMatrices(view, proj);
		Eigen::Matrix4f mvp = proj * view;

		renderer.Render(view, proj, shadingBtn->selectedIndex() == 1, hasParametrization);

		if (chkRenderSecondMesh->checked())
		{
			renderer.Render(view * secondMeshTransform.matrix(), proj, shadingBtn->selectedIndex() == 1, false, Eigen::Vector4f(0.2f, 0.3f, 0.4f, 1.0f));
			if (correspondences.size() > 0)
			{
				corrVAO.bind();
				ShaderPool::Instance()->simpleShader.bind();
				ShaderPool::Instance()->simpleShader.setUniform("color", Eigen::Vector4f(1, 1, 1, 1));
				ShaderPool::Instance()->simpleShader.setUniform("mvp", mvp);
				glDrawArrays(GL_LINES, 0, (GLsizei)correspondences.size() * 2);
				corrVAO.unbind();
			}
		}

		if (hasParametrization && chkRenderTextureMap->checked())
		{
			glDisable(GL_DEPTH_TEST);
			renderer.RenderTextureMap(texMapProjectionMatrix, Eigen::Vector4f(1, 1, 1, 1));
		}
	}
}