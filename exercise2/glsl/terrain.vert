// This source code is property of the Computer Graphics and Visualization
// chair of the TU Dresden. Do not distribute! 
// Copyright (C) CGV TU Dresden - All Rights Reserved

#version 430

in vec4 position;

out vec3 TCS_position;

void main()
{
    // Pass through position to Tessellation Control Shader
    // But first, offset the position in terms of instanced rendering
    TCS_position = position.xyz;
}
