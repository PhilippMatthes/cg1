// This source code is property of the Computer Graphics and Visualization
// chair of the TU Dresden. Do not distribute! 
// Copyright (C) CGV TU Dresden - All Rights Reserved

#version 330

out vec4 color;

uniform vec3 cameraPos;

in vec3 normals;

in vec4 vertexPosition;

uniform sampler2D grassTexture;
uniform sampler2D rockTexture;
uniform sampler2D roadColorTexture;
uniform sampler2D alphaMap;

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

    // TODO: use appropriately scaledxzcoordinate of the fragment in world space as texture coordinates.
	vec2 textureCoordinates = vertexPosition.xz * 10/255;

    //For Oren-Nayar lighting, uncomment the following:
    //Based on: https://stackoverflow.com/questions/40583715/oren-nayar-lighting-in-opengl-how-to-calculate-view-direction-in-fragment-shade#40596525
	//dirToViewer = normalize(vec3(-(gl_FragCoord.xy - screenSize/2) / (screenSize/4), 1.0));

	//material properties

    // Task 2.2.4 + 2.2.3
    // Based on: http://thedemonthrone.ca/projects/rendering-terrain/rendering-terrain-part-23-height-and-slope-based-colours/

	vec3 alphaMapColor = texture(alphaMap, vertexPosition.xz / 255).xyz;

	if(alphaMapColor==vec3(0, 0, 0)){

        float slope = acos(normals.z);
        float blend = (slope - 0.25f) * (1.0f / (0.5f - 0.25f));

        color = mix(texture(grassTexture, textureCoordinates), texture(rockTexture, textureCoordinates), blend);

	} else {

        color = texture(roadColorTexture, textureCoordinates);
        // uncomment the statement below, to make the road even more visible
        //color= color * 2;

	}

	float specular = 0.3;

	//Calculate light
	color = calculateLighting(color, specular, normals, dirToViewer);
	
}