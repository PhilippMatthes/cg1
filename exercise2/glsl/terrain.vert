// This source code is property of the Computer Graphics and Visualization
// chair of the TU Dresden. Do not distribute! 
// Copyright (C) CGV TU Dresden - All Rights Reserved

#version 430

in vec4 position;
in vec4 offset;

out vec3 TCS_position;

uniform mat4 mvp;
uniform mat4 mv;
uniform mat4 projection;

void main()
{
    vec4 offsetPosition = vec4(position.x + offset.x, position.y, position.z + offset.z, position.w);
    gl_Position = (mvp * position);
    TCS_position = (mv * position).xyz;
}
