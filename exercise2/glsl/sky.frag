// This source code is property of the Computer Graphics and Visualization 
// chair of the TU Dresden. Do not distribute! 
// Copyright (C) CGV TU Dresden - All Rights Reserved
#version 330

out vec4 color;

in vec4 clipPos;

const vec4 horizon = vec4(0.85, 0.85, 0.8, 1.0);
const vec4 floor = vec4(0.1, 0.1, 0.1, 1.0);
const vec4 sky = vec4(0.5, 0.6, 0.8, 1.0);

void main()
{
	float h = normalize(clipPos.xyz).y;
	if(h < 0)
		color = mix(horizon, floor, pow(-h, 0.5));
	else
		color = mix(horizon, sky, pow(h, 0.9));
}