// This source code is property of the Computer Graphics and Visualization 
// chair of the TU Dresden. Do not distribute! 
// Copyright (C) CGV TU Dresden - All Rights Reserved

#include "Viewer.h"

#include <nanogui/window.h>
#include <nanogui/layout.h>
#include <nanogui/checkbox.h>

#include <gui/SliderHelper.h>

#include <iostream>

#include <stb_image.h>

#include "glsl.h"
#include "textures.h"

const uint32_t PATCH_SIZE = 256;

Viewer::Viewer()
	: AbstractViewer("CG1 Exercise 2"),
	positionBuffer(nse::gui::VertexBuffer), offsetBuffer(nse::gui::VertexBuffer)
{
	LoadShaders();
	CreateGeometry();

	animation = 0.0;
	
	//Create a texture and framebuffer for the background
	glGenFramebuffers(1, &backgroundFBO);	
	glGenTextures(1, &backgroundTexture);

	// The following steps are necessary on linux, otherwise the background
	// will not attach correctly
	glBindFramebuffer(GL_FRAMEBUFFER, backgroundFBO);
	glBindTexture(GL_TEXTURE_2D, backgroundTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width(), height(), 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, backgroundTexture, 0);
	auto fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Warning: Background framebuffer is not complete: " << fboStatus << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	auto mainWindow = SetupMainWindow();

	sldPerlin1Frequency = nse::gui::AddLabeledSliderWithDefaultDisplay(mainWindow, "Perlin 1 Frequency", std::make_pair(0.001f, 0.05f), 0.02f, 2);
	sldPerlin2Frequency = nse::gui::AddLabeledSliderWithDefaultDisplay(mainWindow, "Perlin 2 Frequency", std::make_pair(0.001f, 0.05f), 0.01f, 2);
	sldPerlin1Height = nse::gui::AddLabeledSliderWithDefaultDisplay(mainWindow, "Perlin 1 Height", std::make_pair(0.0f, 4.0f), 4.0f, 2);
	sldPerlin2Height = nse::gui::AddLabeledSliderWithDefaultDisplay(mainWindow, "Perlin 2 Height", std::make_pair(0.0f, 4.0f), 1.95f, 2);
    sldWaterHeight = nse::gui::AddLabeledSliderWithDefaultDisplay(mainWindow, "Water Height", std::make_pair(0.0f, 10.0f), 0.1f, 2);
	sldSnowHeight = nse::gui::AddLabeledSliderWithDefaultDisplay(mainWindow, "Water Height", std::make_pair(0.0f, 20.0f), 10.0f, 2);
	sldLOD = nse::gui::AddLabeledSliderWithDefaultDisplay(mainWindow, "Tessellation level", std::make_pair(1.0f, 128.0f), 128.0f, 2);

	performLayout();

	//Align camera to view a reasonable part of the terrain
	camera().SetSceneExtent(nse::math::BoundingBox<float, 3>(Eigen::Vector3f(0, 0, 0), Eigen::Vector3f(PATCH_SIZE - 1, 0, PATCH_SIZE - 1)));
	camera().FocusOnPoint(0.5f * Eigen::Vector3f(PATCH_SIZE - 1, 15, PATCH_SIZE - 1));	
	camera().Zoom(-30);
	camera().RotateAroundFocusPointLocal(Eigen::AngleAxisf(-0.5f, Eigen::Vector3f::UnitY()) * Eigen::AngleAxisf(-0.05f, Eigen::Vector3f::UnitX()));
	camera().FixClippingPlanes(0.1, 1000);
}

bool Viewer::resizeEvent(const Eigen::Vector2i&)
{
	//Re-generate the texture and FBO for the background
	glBindFramebuffer(GL_FRAMEBUFFER, backgroundFBO);
	glBindTexture(GL_TEXTURE_2D, backgroundTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width(), height(), 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, backgroundTexture, 0);
	auto fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Warning: Background framebuffer is not complete: " << fboStatus << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return false;
}

void Viewer::LoadShaders()
{
	skyShader.init("Sky Shader", std::string((const char*)sky_vert, sky_vert_size), std::string((const char*)sky_frag, sky_frag_size));
	// terrainShader.init("Terrain Shader", std::string((const char*)terrain_vert, terrain_vert_size), std::string((const char*)terrain_frag, terrain_frag_size));

	terrainShader.initWithTessellation(
	        "Terrain Shader",
            std::string((const char*)terrain_vert, terrain_vert_size),
            std::string((const char*)terrain_tesc, terrain_tesc_size),
            std::string((const char*)terrain_tese, terrain_tese_size),
            std::string((const char*)terrain_frag, terrain_frag_size)
        );
}

unsigned int loadCubemap()
{

    std::vector<unsigned char*> faces =
        {
            right_jpg,
            left_jpg,
            top_jpg,
            bottom_jpg,
            front_jpg,
            back_jpg
        };

    std::vector<size_t> sizes = {
        right_jpg_size,
        left_jpg_size,
        top_jpg_size,
        bottom_jpg_size,
        front_jpg_size,
        back_jpg_size
    };

    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    for (unsigned int i = 0; i < faces.size(); i++)
    {
        int textureWidth, textureHeight, textureChannels;
        auto data = stbi_load_from_memory(faces[i], sizes[i], &textureWidth, &textureHeight, &textureChannels, 3);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0, GL_RGB, textureWidth, textureHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

GLuint CreateTexture(const unsigned char* fileData, size_t fileLength, bool repeat = true)
{

	std::cout << "Loading texture ..." << std::endl;

	//width, height, and number of channels of the loaded texture
	int textureWidth, textureHeight, textureChannels;
	auto pixelData = stbi_load_from_memory(fileData, fileLength, &textureWidth, &textureHeight, &textureChannels, 3);

	// Based on: https://learnopengl.com/Getting-started/Textures
	// & https://github.com/NSchertler/CG1/
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

    if (repeat) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    }
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

	if (pixelData)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureWidth, textureHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, pixelData);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(pixelData);

	return tex;
}

void Viewer::PrintAttributes(GLuint program)
{
	GLint i;
	GLint count;

	GLint size; // size of the variable
	GLenum type; // type of the variable (float, vec3 or mat4, etc)

	const GLsizei bufSize = 16; // maximum name length
	GLchar name[bufSize]; // variable name in GLSL
	GLsizei length; // name length

	glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &count);
	printf("Active Attributes: %d\n", count);

	for (i = 0; i < count; i++)
	{
		glGetActiveAttrib(program, (GLuint)i, bufSize, &length, &size, &type, name);

		printf("Attribute #%d Type: %u Name: %s\n", i, type, name);
	}
}

void Viewer::CreateGeometry()
{
	//empty VAO for sky
	emptyVAO.generate();

	//terrain VAO	
	terrainVAO.generate();
	terrainVAO.bind();

	terrainShader.bind();
	PrintAttributes(terrainShader.mProgramShader);

	// Task 2.2.5 a)
	// See: https://tu-dresden.de/ing/informatik/smt/cgv/ressourcen/dateien/lehre/ws-18-19/cg1/CGI_03_Geometry.pdf?lang=de
	offsetBuffer.bind();
	GLuint offset = static_cast<GLuint>(terrainShader.attrib("offset"));
	glEnableVertexAttribArray(offset);
	glVertexAttribPointer(offset, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribDivisor(offset, 1);

	std::vector<Eigen::Vector4f> positions;

    for (int x = 0; x <= 3; x += 1) {
        for (int y = 0; y <= 3; y += 1) {
        	int scale = PATCH_SIZE / 3;
            positions.emplace_back(x * scale, 0, y * scale, 1);
        }
    }

	positionBuffer.bind();
	positionBuffer.uploadData(positions).bindToAttribute("position");

	std::cout << "Creating textures ..." << std::endl;

	//textures
	grassTexture = CreateTexture((unsigned char*)grass_jpg, grass_jpg_size);
	rockTexture = CreateTexture((unsigned char*)rock_jpg, rock_jpg_size);
	roadColorTexture = CreateTexture((unsigned char*)roadcolor_jpg, roadcolor_jpg_size);
	roadNormalMap = CreateTexture((unsigned char*)roadnormals_jpg, roadnormals_jpg_size);
	roadSpecularMap = CreateTexture((unsigned char*)roadspecular_jpg, roadspecular_jpg_size);
	waterNormalMap = CreateTexture((unsigned char*)waternormals_jpg, waternormals_jpg_size);
	waterTexture = CreateTexture((unsigned char*)watertexture_jpg, watertexture_jpg_size);
	snowTexture = CreateTexture((unsigned char*)snow_jpg, snow_jpg_size);
    snowNormalMap = CreateTexture((unsigned char*)snownormals_jpg, snownormals_jpg_size);

    skybox = loadCubemap();

	// Road texture should not repeat itself
	alphaMap = CreateTexture((unsigned char*)alpha_jpg, alpha_jpg_size, false);
}

void Viewer::RenderSky()
{
	Eigen::Matrix4f skyView = view;
	for (int i = 0; i < 3; ++i)
		skyView.col(i).normalize();
	skyView.col(3).head<3>().setZero();
	Eigen::Matrix4f skyMvp = proj * skyView;
	glDepthMask(GL_FALSE);
	glEnable(GL_DEPTH_CLAMP);
	emptyVAO.bind();
	skyShader.bind();
	skyShader.setUniform("mvp", skyMvp);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);
	glDisable(GL_DEPTH_CLAMP);
	glDepthMask(GL_TRUE);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, backgroundFBO);
	glBlitFramebuffer(0, 0, width(), height(), 0, 0, width(), height(), GL_COLOR_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void CalculateViewFrustum(const Eigen::Matrix4f mvp, Eigen::Vector4f* frustumPlanes, nse::math::BoundingBox<float, 3>& bbox)
{
	frustumPlanes[0] = (mvp.row(3) + mvp.row(0)).transpose();
    frustumPlanes[1] = (mvp.row(3) - mvp.row(0)).transpose();
    frustumPlanes[2] = (mvp.row(3) + mvp.row(1)).transpose();
    frustumPlanes[3] = (mvp.row(3) - mvp.row(1)).transpose();
    frustumPlanes[4] = (mvp.row(3) + mvp.row(2)).transpose();
    frustumPlanes[5] = (mvp.row(3) - mvp.row(2)).transpose();

	Eigen::Matrix4f invMvp = mvp.inverse();
	bbox.reset();
	for(int x = -1; x <= 1; x += 2)
		for(int y = -1; y <= 1; y += 2)
			for (int z = -1; z <= 1; z += 2)
	{
		Eigen::Vector4f corner = invMvp * Eigen::Vector4f(x, y, z, 1);
		corner /= corner.w();
		bbox.expand(corner.head<3>());
	}
}

bool IsBoxCompletelyBehindPlane(const Eigen::Vector3f& boxMin, const Eigen::Vector3f& boxMax, const Eigen::Vector4f& plane)
{
	return
		plane.dot(Eigen::Vector4f(boxMin.x(), boxMin.y(), boxMin.z(), 1)) < 0 &&
		plane.dot(Eigen::Vector4f(boxMin.x(), boxMin.y(), boxMax.z(), 1)) < 0 &&
		plane.dot(Eigen::Vector4f(boxMin.x(), boxMax.y(), boxMin.z(), 1)) < 0 &&
		plane.dot(Eigen::Vector4f(boxMin.x(), boxMax.y(), boxMin.z(), 1)) < 0 &&
		plane.dot(Eigen::Vector4f(boxMax.x(), boxMin.y(), boxMin.z(), 1)) < 0 &&
		plane.dot(Eigen::Vector4f(boxMax.x(), boxMin.y(), boxMax.z(), 1)) < 0 &&
		plane.dot(Eigen::Vector4f(boxMax.x(), boxMax.y(), boxMin.z(), 1)) < 0 &&
		plane.dot(Eigen::Vector4f(boxMax.x(), boxMax.y(), boxMin.z(), 1)) < 0;
}

void Viewer::drawContents()
{
	camera().ComputeCameraMatrices(view, proj);

	animation += 0.03;

	// Task 2.2.5 b)
	Eigen::Matrix4f mvp = proj * view;
	Eigen::Vector3f cameraPosition = view.inverse().col(3).head<3>();

	RenderSky();

	//render terrain
	glEnable(GL_DEPTH_TEST);
	terrainVAO.bind();
	terrainShader.bind();

	auto *frustumPlanes = new Eigen::Vector4f[6];
	nse::math::BoundingBox<float, 3> boundingBox;

	// Calculate view frustum planes
	CalculateViewFrustum(mvp, frustumPlanes, boundingBox);

	// Create all patches intersecting with the bounding box
	// and check, if they are visible. If so, add them to the
	// offsets vector and increment visiblePatches.
	int minX = (int) boundingBox.min[0];
	int minY = 0;
	int minZ = (int) boundingBox.min[2];
	int maxX = (int) boundingBox.max[0];
	int maxY = 15;
	int maxZ = (int) boundingBox.max[2];

	int clampedMinX = minX - (minX % PATCH_SIZE);
	int clampedMinZ = minZ - (minZ % PATCH_SIZE);
	int clampedMaxX = maxX - (maxX % PATCH_SIZE);
	int clampedMaxZ = maxZ - (maxZ % PATCH_SIZE);

	int visiblePatches = 0;

	std::vector<Eigen::Vector4f> offsets;

	for (int x = clampedMinX; x <= clampedMaxX; x += PATCH_SIZE) {
		for (int z = clampedMinZ; z <= clampedMaxZ; z += PATCH_SIZE) {
			nse::math::BoundingBox<float, 3> patchBox (
			  Eigen::Matrix<float, 3, 1> ((float) x, (float) minY, (float) z),
			  Eigen::Matrix<float, 3, 1> ((float) (x + PATCH_SIZE), (float) maxY, (float) (z + PATCH_SIZE))
			);

			bool isBehind = false;
			for (int p = 0; p < 6; p += 1) {
				if (IsBoxCompletelyBehindPlane(patchBox.min, patchBox.max, frustumPlanes[p])) {
					isBehind = true;
					break;
				}
			}
			if (!isBehind) {
				offsets.emplace_back((float) x, 0, (float) z, 1);

				visiblePatches += 1;
			}
		}
	}

	offsetBuffer.uploadData(offsets);

	terrainShader.setUniform("mvp", mvp, false);
	terrainShader.setUniform("screenSize", Eigen::Vector2f(width(), height()));
	terrainShader.setUniform("mv", view, false);
	terrainShader.setUniform("projection", proj, false);
	terrainShader.setUniform("cameraPos", cameraPosition);

    terrainShader.setUniform("animation", animation);

	terrainShader.setUniform("perlinNoise1Frequency", sldPerlin1Frequency->value());
	terrainShader.setUniform("perlinNoise2Frequency", sldPerlin2Frequency->value());
	terrainShader.setUniform("perlinNoise1Height", sldPerlin1Height->value());
	terrainShader.setUniform("perlinNoise2Height", sldPerlin2Height->value());
    terrainShader.setUniform("waterHeight", sldWaterHeight->value());
	terrainShader.setUniform("snowHeight", sldSnowHeight->value());
	terrainShader.setUniform("lod", sldLOD->value());

	/* Task: Render the terrain */
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, grassTexture);
	terrainShader.setUniform("grassTexture", 0, false);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, rockTexture);
	terrainShader.setUniform("rockTexture", 1, false);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, roadColorTexture);
	terrainShader.setUniform("roadColorTexture", 2, false);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, alphaMap);
	terrainShader.setUniform("alphaMap", 3, false);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, roadSpecularMap);
	terrainShader.setUniform("roadSpecularMap", 4, false);

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, roadNormalMap);
	terrainShader.setUniform("roadNormalMap", 5, false);

	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, backgroundTexture);
	terrainShader.setUniform("background", 6, false);

	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox);
	terrainShader.setUniform("skybox", 7, false);

	glActiveTexture(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_2D, waterNormalMap);
	terrainShader.setUniform("waterNormalMap", 8, false);

	glActiveTexture(GL_TEXTURE9);
	glBindTexture(GL_TEXTURE_2D, waterTexture);
	terrainShader.setUniform("waterTexture", 9, false);

    glActiveTexture(GL_TEXTURE10);
    glBindTexture(GL_TEXTURE_2D, snowTexture);
    terrainShader.setUniform("snowTexture", 10, false);

	glActiveTexture(GL_TEXTURE11);
	glBindTexture(GL_TEXTURE_2D, snowNormalMap);
	terrainShader.setUniform("snowNormalMap", 11, false);

	glClearDepth(1);
	glEnable(GL_DEPTH_TEST);

	glPatchParameteri(GL_PATCH_VERTICES, 16);

	// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	// glDrawArrays(GL_PATCHES, 0, 4);
	glDrawArraysInstanced(GL_PATCHES, 0, 16, visiblePatches);
	// glDrawElementsInstanced(GL_PATCHES, visiblePatches, GL_UNSIGNED_INT, 0, visiblePatches);
	
	//Render text
	nvgBeginFrame(mNVGContext, width(), height(), mPixelRatio);
	std::string text = "Patches visible: " + std::to_string(visiblePatches);
	nvgText(mNVGContext, 10, 20, text.c_str(), nullptr);
	nvgEndFrame(mNVGContext);
}
