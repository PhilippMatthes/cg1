// This source code is property of the Computer Graphics and Visualization 
// chair of the TU Dresden. Do not distribute! 
// Copyright (C) CGV TU Dresden - All Rights Reserved

#include <gui/ShaderPool.h>
#include "glsl.h"

ShaderPool* ShaderPool::_instance(nullptr);
ShaderPool* ShaderPool::Instance()
{
	if (_instance == nullptr)
	{
		_instance = new ShaderPool();
		_instance->CompileShaders();
	}

	return _instance;
}

void ShaderPool::CompileShaders()
{
	meshShader.init("Mesh Shader", std::string((char*)mesh_vert, mesh_vert_size), std::string((char*)mesh_frag, mesh_frag_size));
    ssaoShader.init("SSAO Shader", std::string((char*)ssao_vert, ssao_vert_size), std::string((char*)ssao_frag, ssao_frag_size));
    geometryShader.init("Geometry Shader", std::string((char*)geometry_vert, geometry_vert_size), std::string((char*)geometry_frag, geometry_frag_size));
	simpleShader.init("Simple Shader", std::string((char*)simple_vert, simple_vert_size), std::string((char*)simple_frag, simple_frag_size));
}