#pragma once

#include <Eigen/Core>
#include <OpenMesh/Core/Mesh/PolyMesh_ArrayKernelT.hh>

#include <gui/GLBuffer.h>
#include <gui/GLVertexArray.h>
#include <gui/GLShader.h>

typedef OpenMesh::PolyMesh_ArrayKernelT<>  HEMesh;

//Converts an OpenMesh vector to an Eigen vector 
static Eigen::Vector3f ToEigenVector(const OpenMesh::Vec3f& v) 
{ 
	return Eigen::Vector3f(v[0], v[1], v[2]); 
} 

//Converts an OpenMesh vector to an Eigen vector 
static Eigen::Vector2f ToEigenVector(const OpenMesh::Vec2f& v)
{
	return Eigen::Vector2f(v[0], v[1]);
}

//Converts an Eigen vector to an OpenMesh vector 
static OpenMesh::Vec3f ToOpenMeshVector(const Eigen::Vector3f& v)
{
	return OpenMesh::Vec3f(v[0], v[1], v[2]);
}

//Converts an OpenMesh vector to an Eigen vector 
static Eigen::Vector4f ToEigenVector4(const OpenMesh::Vec3f& v, float w = 1)
{
	return Eigen::Vector4f(v[0], v[1], v[2], w);
}

//Converts an OpenMesh vector to an Eigen vector 
static Eigen::Vector4f ToEigenVector4(const OpenMesh::Vec2f& v, float z = 0, float w = 1)
{
	return Eigen::Vector4f(v[0], v[1], z, w);
}

//GPU representation of a mesh with rendering capabilities.
class MeshRenderer
{
public:
	MeshRenderer(const HEMesh& mesh);

	//Update the underlying buffers based on the current geometry in the referenced mesh
	void Update();

    GLuint gBuffer;
    GLuint gPosition, gNormal, gAlbedo;

    //float MeshRenderer::lerp(float a, float b, float f);
	void UpdateWithPerFaceColor(OpenMesh::FPropHandleT<Eigen::Vector4f> colorProperty);
    //void MeshRenderer::UpdateSSAOBuffers();
    std::vector<Eigen::Vector3f> ssaoKernel;
    GLuint noiseTexture;

    void Render(const Eigen::Matrix4f& view, const Eigen::Matrix4f& projection, bool flatShading = false, bool withTexCoords = false, const Eigen::Vector4f& color = Eigen::Vector4f(0.8f, 0.7f, 0.6f, 1.0f)) const;
	void RenderGBuffer(const Eigen::Matrix4f& view, const Eigen::Matrix4f& projection, bool flatShading = false, bool withTexCoords = false, const Eigen::Vector4f& color = Eigen::Vector4f(0.8f, 0.7f, 0.6f, 1.0f)) const;
	void RenderSSAO(const Eigen::Matrix4f& view, const Eigen::Matrix4f& projection, bool flatShading = false, bool withTexCoords = false, const Eigen::Vector4f& color = Eigen::Vector4f(0.8f, 0.7f, 0.6f, 1.0f)) const;
	void RenderTextureMap(const Eigen::Matrix4f& projection, const Eigen::Vector4f& color) const;

private:	

	void UpdateTextureMapBuffers();

	const HEMesh& mesh;

	nse::gui::GLBuffer positionBuffer, normalBuffer, colorBuffer, texCoordBuffer, texCoordBuffer4D;
	nse::gui::GLBuffer indexBuffer, indexBufferTexCoords;
	unsigned int indexCount;
	unsigned int indexCountTexCoords;
	nse::gui::GLVertexArray vao, vaoTexCoords;

	bool hasColor = false;
};