// This source code is property of the Computer Graphics and Visualization
// chair of the TU Dresden. Do not distribute! 
// Copyright (C) CGV TU Dresden - All Rights Reserved

#version 330

out vec4 color;

uniform vec3 cameraPos;

in vec3 normals;

in vec4 vertexPosition;

in float fogVisibility;

uniform sampler2D grassTexture;
uniform sampler2D rockTexture;
uniform sampler2D roadColorTexture;
uniform sampler2D alphaMap;
uniform sampler2D roadSpecularMap;
uniform sampler2D roadNormalMap;

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

	vec2 textureCoordinates = vertexPosition.xz * 10/255;

    //For Oren-Nayar lighting, uncomment the following:
    //Based on: https://stackoverflow.com/questions/40583715/oren-nayar-lighting-in-opengl-how-to-calculate-view-direction-in-fragment-shade#40596525
	dirToViewer = normalize(vec3(-(gl_FragCoord.xy - screenSize/2) / (screenSize/4), 1.0));

	//material properties

    // Tasks 2.2.4 + 2.2.3
    // Based on: http://thedemonthrone.ca/projects/rendering-terrain/rendering-terrain-part-23-height-and-slope-based-colours/

    // Calculate terrain color
	float slope = acos(normals.z);
    float blend = (length(slope) - 0.25f) * (1.0f / (0.5f - 0.25f));
    vec4 terrainColor = mix(texture(grassTexture, textureCoordinates), texture(rockTexture, textureCoordinates), blend);

    // Calculate alpha map
    vec3 alphaMapColor = texture(alphaMap, vertexPosition.xz / 255).xyz;

    // Calculate road normals. Use the slope (bitangent) and the tangent
    // given in the normal map to get our normal (by cross product)
    vec3 tangent = vec3(texture(roadNormalMap, textureCoordinates).rgb);
    vec3 inverseBitangent = normalize(acos(normals));
    vec3 bitangent = vec3(inverseBitangent.x, -inverseBitangent.y, inverseBitangent.z);
    vec3 roadNormals = normalize(cross(tangent, bitangent));

    // Calculate road color
    vec4 roadColor = texture(roadColorTexture, textureCoordinates);
    roadColor = vec4((alphaMapColor * roadColor.xyz), roadColor.w);

    // Calculate road specular
    vec3 specularVector = vec3(texture(roadSpecularMap, textureCoordinates));
    float roadSpecular = specularVector.x;

    // Blend color, normals and specular together
    color = mix(terrainColor, roadColor, alphaMapColor.x);
    vec3 blendedNormals = mix(normals, roadNormals, alphaMapColor.x);
    float blendedSpecular = alphaMapColor.x * roadSpecular;

	// Calculate light
	color = calculateLighting(color, blendedSpecular, blendedNormals, dirToViewer);

	// Calculate fog
	color = mix(color, getBackgroundColor(), fogVisibility);

    // Uncomment for specular testing:
    // color = vec4(blendedSpecular, blendedSpecular, blendedSpecular, blendedSpecular);

    // Uncomment for normal testing:
    // color = calculateLighting(vec4(1.0, 1.0, 1.0, 1.0), blendedSpecular, blendedNormals, dirToViewer);
	
}