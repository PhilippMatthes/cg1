// This source code is property of the Computer Graphics and Visualization
// chair of the TU Dresden. Do not distribute! 
// Copyright (C) CGV TU Dresden - All Rights Reserved

#version 430

in vec2 position;
in vec2 offset;

out vec3 TCS_position;

uniform mat4 mvp;
uniform mat4 mv;
uniform mat4 projection;

void main()
{
    vec4 offsetPosition = vec4(position.x + offset.x, 0, position.y + offset.y, 1);
    gl_Position = (mvp * offsetPosition);
    TCS_position = (mv * offsetPosition).xyz;
}
