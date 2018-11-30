// This source code is property of the Computer Graphics and Visualization
// chair of the TU Dresden. Do not distribute! 
// Copyright (C) CGV TU Dresden - All Rights Reserved

#version 330

in vec4 position;
in vec4 offset;

void main()
{
    // Pass through position to Tessellation Control Shader
    // But first, offset the position in terms of instanced rendering
    vec4 offsetPosition = vec4(position.x + offset.x, position.y, position.z + offset.y, position.w);
    gl_Position = offsetPosition;
}
