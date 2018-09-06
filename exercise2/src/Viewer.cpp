// This source code is property of the Computer Graphics and Visualization 
// chair of the TU Dresden. Do not distribute! 
// Copyright (C) CGV TU Dresden - All Rights Reserved

#include "Viewer.h"

#include <nanogui/window.h>
#include <nanogui/button.h>
#include <nanogui/checkbox.h>
#include <nanogui/messagedialog.h>
#include <nanogui/popupbutton.h>
#include <nanogui/layout.h>
#include <nanogui/combobox.h>

#include <iostream>

#include <OpenMesh/Core/IO/MeshIO.hh>

#include <gui/SliderHelper.h>

#include "Primitives.h"
#include "SurfaceArea.h"
#include "Volume.h"
#include "ShellExtraction.h"
#include "Smoothing.h"
#include "Stripification.h"

const int segmentColorCount = 12;
const float segmentColors[segmentColorCount][3] =
{
	{ 0.651f, 0.808f, 0.890f },
	{ 0.122f, 0.471f, 0.706f },
	{ 0.698f, 0.875f, 0.541f },
	{ 0.200f, 0.627f, 0.173f },
	{ 0.984f, 0.604f, 0.600f },
	{ 0.890f, 0.102f, 0.110f },
	{ 0.992f, 0.749f, 0.435f },
	{ 1.000f, 0.498f, 0.000f },
	{ 0.792f, 0.698f, 0.839f },
	{ 0.416f, 0.239f, 0.604f },
	{ 1.000f, 1.000f, 0.600f },
	{ 0.694f, 0.349f, 0.157f },

};

Viewer::Viewer()
	: AbstractViewer("CG1 Exercise 2"),
	renderer(polymesh)
{ 
	SetupGUI();	

	polymesh.add_property(faceIdProperty);
	polymesh.add_property(faceColorProperty);
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
				MeshUpdated(true);
		}
	});

	auto primitiveBtn = new nanogui::PopupButton(mainWindow, "Create Primitive");
	primitiveBtn->popup()->setLayout(new nanogui::BoxLayout(nanogui::Orientation::Vertical, nanogui::Alignment::Fill, 4, 4));

	auto quadBtn = new nanogui::Button(primitiveBtn->popup(), "Quad");
	quadBtn->setCallback([this]() { CreateQuad(polymesh); MeshUpdated(true); });

	auto diskBtn = new nanogui::Button(primitiveBtn->popup(), "Disk");
	diskBtn->setCallback([this]() { CreateDisk(polymesh, 1, 20); MeshUpdated(true); });

	auto tetBtn = new nanogui::Button(primitiveBtn->popup(), "Tetrahedron");
	tetBtn->setCallback([this]() { CreateTetrahedron(polymesh); MeshUpdated(true); });

	auto octaBtn = new nanogui::Button(primitiveBtn->popup(), "Octahedron");
	octaBtn->setCallback([this]() { CreateOctahedron(polymesh, 1); MeshUpdated(true); });

	auto cubeBtn = new nanogui::Button(primitiveBtn->popup(), "Cube");
	cubeBtn->setCallback([this]() { CreateCube(polymesh); MeshUpdated(true); });

	auto icoBtn = new nanogui::Button(primitiveBtn->popup(), "Icosahedron");
	icoBtn->setCallback([this]() { CreateIcosahedron(polymesh, 1); MeshUpdated(true); });

	auto cylBtn = new nanogui::Button(primitiveBtn->popup(), "Cylinder");
	cylBtn->setCallback([this]() { CreateCylinder(polymesh, 0.3f, 1, 20, 10); MeshUpdated(true); });

	auto sphereBtn = new nanogui::Button(primitiveBtn->popup(), "Sphere");
	sphereBtn->setCallback([this]() { CreateSphere(polymesh, 1, 20, 20); MeshUpdated(true); });

	auto torusBtn = new nanogui::Button(primitiveBtn->popup(), "Torus");
	torusBtn->setCallback([this]() { CreateTorus(polymesh, 0.4f, 1, 20, 20); MeshUpdated(true); });

	auto arrowBtn = new nanogui::Button(primitiveBtn->popup(), "Arrow");
	arrowBtn->setCallback([this]() { CreateUnitArrow(polymesh); MeshUpdated(true); });

	auto calcAreaBtn = new nanogui::Button(mainWindow, "Calculate Mesh Area");
	calcAreaBtn->setCallback([this]() {
		auto area = ComputeSurfaceArea(polymesh);
		std::stringstream ss;
		ss << "The mesh has an area of " << area << ".";
		new nanogui::MessageDialog(this, nanogui::MessageDialog::Type::Information, "Surface Area",
			ss.str());
	});

	auto calcVolBtn = new nanogui::Button(mainWindow, "Calculate Mesh Volume");
	calcVolBtn->setCallback([this]() {
		//Triangulate the mesh if it is not a triangle mesh
		for (auto f : polymesh.faces())
		{
			if (polymesh.valence(f) > 3)
			{
				std::cout << "Triangulating mesh." << std::endl;
				polymesh.triangulate();
				MeshUpdated();
				break;
			}
		}
		
		auto vol = ComputeVolume(polymesh);
		std::stringstream ss;
		ss << "The mesh has a volume of " << vol << ".";
		new nanogui::MessageDialog(this, nanogui::MessageDialog::Type::Information, "Volume",
			ss.str());
	});

	auto extractShellsBtn = new nanogui::Button(mainWindow, "Extract Shells");
	extractShellsBtn->setCallback([this]() {
		auto count = ExtractShells(polymesh, faceIdProperty);
		std::stringstream ss;
		ss << "The mesh has " << count << " shells.";
		new nanogui::MessageDialog(this, nanogui::MessageDialog::Type::Information, "Shell Extraction",
			ss.str());

		ColorMeshFromIds();
	});

	auto noiseBtn = new nanogui::Button(mainWindow, "Add Noise");
	noiseBtn->setCallback([this]() { AddNoise(polymesh); MeshUpdated(); });

	nanogui::TextBox* txtSmoothingIterations;
	auto sldSmoothingIterations = nse::gui::AddLabeledSlider(mainWindow, "Smoothing Iterations", std::make_pair(1, 100), 20, txtSmoothingIterations);
	sldSmoothingIterations->setCallback([this, txtSmoothingIterations](float value)
	{
		smoothingIterations = (unsigned int)std::round(value);
		txtSmoothingIterations->setValue(std::to_string(smoothingIterations));
	});
	sldSmoothingIterations->callback()(sldSmoothingIterations->value());

	sldSmoothingStrength = nse::gui::AddLabeledSliderWithDefaultDisplay(mainWindow, "Smoothing Strength", std::make_pair(0.0f, 1.0f), 0.1f, 2);

	
	auto smoothBtn = new nanogui::Button(mainWindow, "Laplacian Smoothing");
	smoothBtn->setCallback([this]() {
		SmoothUniformLaplacian(polymesh, sldSmoothingStrength->value(), smoothingIterations);
		MeshUpdated();
	});

	nanogui::TextBox* txtStripificationTrials;
	auto sldStripificationTrials = nse::gui::AddLabeledSlider(mainWindow, "Stripification Trials", std::make_pair(1, 50), 20, txtStripificationTrials);
	sldStripificationTrials->setCallback([this, txtStripificationTrials](float value)
	{
		stripificationTrials = (unsigned int)std::round(value);
		txtStripificationTrials->setValue(std::to_string(stripificationTrials));
	});
	sldStripificationTrials->callback()(sldStripificationTrials->value());

	auto stripifyBtn = new nanogui::Button(mainWindow, "Extract Triangle Strips");
	stripifyBtn->setCallback([this]() {
		//Triangulate the mesh if it is not a triangle mesh
		for (auto f : polymesh.faces())
		{
			if (polymesh.valence(f) > 3)
			{
				std::cout << "Triangulating mesh." << std::endl;
				polymesh.triangulate();
				MeshUpdated();
				break;
			}
		}

		auto count = ExtractTriStrips(polymesh, faceIdProperty, stripificationTrials);
		std::stringstream ss;
		ss << "The mesh has " << count << " triangle strips.";
		new nanogui::MessageDialog(this, nanogui::MessageDialog::Type::Information, "Shell Extraction",
			ss.str());

		ColorMeshFromIds();
	});

	shadingBtn = new nanogui::ComboBox(mainWindow, { "Smooth Shading", "Flat Shading" });

	performLayout();
}

void Viewer::ColorMeshFromIds()
{
	//Set face colors
	for (auto f : polymesh.faces())
	{
		auto shell = polymesh.property(faceIdProperty, f);
		if (shell < 0)
			polymesh.property(faceColorProperty, f) = Eigen::Vector4f(0, 0, 0, 1);
		else
		{
			auto& color = segmentColors[shell % segmentColorCount];
			polymesh.property(faceColorProperty, f) = Eigen::Vector4f(color[0], color[1], color[2], 1);
		}
	}
	hasColors = true;
	MeshUpdated();
}

void Viewer::MeshUpdated(bool initNewMesh)
{
	if (initNewMesh)
	{
		hasColors = false;

		//calculate the bounding box of the mesh
		nse::math::BoundingBox<float, 3> bbox;
		for (auto v : polymesh.vertices())
			bbox.expand(ToEigenVector(polymesh.point(v)));
		camera().FocusOnBBox(bbox);
	}	

	if (hasColors)
		renderer.UpdateWithPerFaceColor(faceColorProperty);
	else
		renderer.Update();
}

void Viewer::drawContents()
{
	glEnable(GL_DEPTH_TEST);

	Eigen::Matrix4f view, proj;
	camera().ComputeCameraMatrices(view, proj);

	renderer.Render(view, proj, shadingBtn->selectedIndex() == 1);
}