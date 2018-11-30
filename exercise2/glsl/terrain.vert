// This source code is property of the Computer Graphics and Visualization
// chair of the TU Dresden. Do not distribute! 
// Copyright (C) CGV TU Dresden - All Rights Reserved

#version 330

in vec2 offset;
out vec3 TEC_Position;

void main()
{
    // Pass through position to Tessellation Control Shader
    // But first, offset the position in terms of instanced rendering
    TESC_Position = vec3(offset.x, 0, offset.y);
}
