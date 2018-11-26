// This source code is property of the Computer Graphics and Visualization
// chair of the TU Dresden. Do not distribute! 
// Copyright (C) CGV TU Dresden - All Rights Reserved

#version 330

in vec3 normals;
in vec4 vertexPosition;

out vec4 color;

uniform sampler2D grassTexture;
uniform sampler2D rockTexture;
uniform sampler2D roadColorTexture;
uniform sampler2D alphaMap;
uniform sampler2D roadSpecularMap;
uniform sampler2D roadNormalMap;

uniform sampler2D background;
uniform vec3 cameraPos;
uniform vec2 screenSize;

uniform bool showNormalMappingOnly;
uniform bool showSpecularLightingOnly;
uniform bool useNormalMap;
uniform bool showFog;

const vec3 dirToLight = normalize(vec3(1, 3, 1));
const float FogDensity = 0.005;
const vec3 fogColor = vec3(0.5, 0.5,0.5);

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

float getFogFactor()
{
    float dist = gl_FragCoord.z / gl_FragCoord.w;
    float fogFactor = 1.0 /exp( (dist * FogDensity)* (dist * FogDensity));
    fogFactor = clamp( fogFactor, 0.0, 1.0 );
    return fogFactor;
}

//Based on: https://github.com/NSchertler/CG1/blob/master/glsl/texturedMesh.frag
void calculateTangentSpace( vec3 N, vec3 p, vec2 uv, out mat3 tangentSpaceToViewSpace, out mat3 viewSpaceToTangentSpace )
{
    // get derivatives of fragment position and texture coordinates with respect to screen position
    vec3 dpdx = dFdx( p );
    vec3 dpdy = dFdy( p );
    vec2 duvdx = dFdx( uv );
    vec2 duvdy = dFdy( uv );

	//Calculate dp/duv
	mat2 m = inverse(mat2(duvdx, duvdy));
	mat2x3 TB = mat2x3(dpdx, dpdy) * m;
	tangentSpaceToViewSpace = mat3(TB[0], TB[1], N);

	//Calculate duv/dp = inverse(dp/duv)
    vec3 dpdyperp = cross( dpdy, N );
    vec3 dpdxperp = cross( N, dpdx );
	float invDet = 1 / dot(cross(dpdx, dpdy), N);
    vec3 T = invDet * ( dpdyperp * duvdx.x + dpdxperp * duvdy.x);
    vec3 B = invDet * ( dpdyperp * duvdx.y + dpdxperp * duvdy.y);

    viewSpaceToTangentSpace = transpose(mat3( T, B, N ));
}

void main()
{
    bool useNormalMap = true;
    vec3 fragmentPosition = vertexPosition.xyz;
	vec2 textureCoordinates = vertexPosition.xz * 10/255;
	vec3 n = normalize(normals);

	mat3 tangentSpaceToViewSpace, viewSpaceToTangentSpace;
	calculateTangentSpace(n, fragmentPosition, textureCoordinates, tangentSpaceToViewSpace, viewSpaceToTangentSpace);

    //surface geometry
    //For Oren-Nayar lighting, uncomment the following:
    //Based on: https://stackoverflow.com/questions/40583715/oren-nayar-lighting-in-opengl-how-to-calculate-view-direction-in-fragment-shade#40596525
    //	vec3 dirToViewer = vec3(0, 1, 0);
	//vec3 dirToViewer = normalize(vec3(-(gl_FragCoord.xy - screenSize/2) / (screenSize/4), 1.0));
    vec3 dirToViewer = normalize(-fragmentPosition); //viewer is at the origin in camera space


	//material properties
    // Task 2.2.4 + 2.2.3
    // Based on: http://thedemonthrone.ca/projects/rendering-terrain/rendering-terrain-part-23-height-and-slope-based-colours/
    // Calculate terrain color
	float slope = acos(normals.z);
    float blend = (length(slope) - 0.25f) * (1.0f / (0.5f - 0.25f));
    vec4 terrainColor = mix(texture(grassTexture, textureCoordinates), texture(rockTexture, textureCoordinates), blend);

    // Calculate alpha map
    vec3 alphaMapColor = texture(alphaMap, vertexPosition.xz / 255).xyz;
    // & use it to smootly Calculate road color
    vec4 roadColor = texture(roadColorTexture, textureCoordinates);
    roadColor = vec4((alphaMapColor * roadColor.xyz), roadColor.w);

    if(useNormalMap)
    {
        vec3 mapNormal = texture(roadNormalMap, textureCoordinates).xyz * 2 - 1;
        mapNormal.y *= -1;
        tangentSpaceToViewSpace[0] = normalize(tangentSpaceToViewSpace[0]);
        tangentSpaceToViewSpace[1] = normalize(tangentSpaceToViewSpace[1]);
        n = normalize(tangentSpaceToViewSpace * mapNormal);
    }

    // Calculate road specular
    vec3 specularVector = vec3(texture(roadSpecularMap, textureCoordinates));
    float roadSpecular = specularVector.x;

    // Blend color, normals and specular together
    color = mix(terrainColor, roadColor, alphaMapColor.x);
    vec3 blendedNormals = mix(normals, n, alphaMapColor.x);
    float blendedSpecular = alphaMapColor.x * roadSpecular;

    if(showSpecularLightingOnly)
         color = vec4(blendedSpecular, blendedSpecular, blendedSpecular, blendedSpecular);
    else if (showNormalMappingOnly)
        color = calculateLighting(vec4(1.0, 1.0, 1.0, 1.0), blendedSpecular, blendedNormals, dirToViewer);
    else if(showFog){
        color = calculateLighting(color, blendedSpecular, blendedNormals, dirToViewer);
        color = mix(vec4(fogColor, 1), color, getFogFactor());
    }
    else
        color = calculateLighting(color, blendedSpecular, blendedNormals, dirToViewer);
}