// This source code is property of the Computer Graphics and Visualization
// chair of the TU Dresden. Do not distribute! 
// Copyright (C) CGV TU Dresden - All Rights Reserved

#version 330

out vec4 color;

uniform vec3 cameraPos;

in vec3 normals;

uniform sampler2D grassTexture;

uniform sampler2D background;

uniform vec2 screenSize;

const vec3 dirToLight = normalize(vec3(1, 3, 1));	

//Calculates the visible surface color based on the Blinn-Phong illumination model
vec4 calculateLighting(vec4 materialColor, float specularIntensity, vec3 normalizedNormal, vec3 directionToViewer)
{
	vec4 color = materialColor;
	vec3 h = normalize(dirToLight + directionToViewer);
	color.xyz *= 0.9 * max(dot(normalizedNormal, dirToLight), 0) + 0.1;
	color.xyz += specularIntensity * pow(max(dot(h, normalizedNormal), 0), 50);
	return color;
}

vec4 getBackgroundColor()
{
	return texture(background, gl_FragCoord.xy / screenSize);
}

void main()
{
	//surface geometry
	vec3 dirToViewer = vec3(0, 1, 0);

    //For Oren-Nayar lighting, uncomment the following:
    //Based on: https://stackoverflow.com/questions/40583715/oren-nayar-lighting-in-opengl-how-to-calculate-view-direction-in-fragment-shade#40596525
	//dirToViewer = normalize(vec3(-(gl_FragCoord.xy - screenSize/2) / (screenSize/4), 1.0));

	//material properties	
	color = texture(grassTexture, gl_FragCoord.xy);
	float specular = 0.3;



	//Calculate light
	color = calculateLighting(color, specular, normals, dirToViewer);

	
}