// This source code is property of the Computer Graphics and Visualization
// chair of the TU Dresden. Do not distribute! 
// Copyright (C) CGV TU Dresden - All Rights Reserved

#version 430

in vec3 FRAG_normals;
in vec3 FRAG_position;
in float FRAG_waterFactor;
in float FRAG_snowFactor;


out vec4 color;

uniform sampler2D grassTexture;
uniform sampler2D rockTexture;
uniform sampler2D roadColorTexture;
uniform sampler2D alphaMap;
uniform sampler2D roadSpecularMap;
uniform sampler2D roadNormalMap;
uniform sampler2D background;
uniform samplerCube skybox;
uniform sampler2D waterTexture;
uniform sampler2D waterNormalMap;
uniform sampler2D snowTexture;
uniform sampler2D snowNormalMap;

uniform float animation;

uniform vec3 cameraPos;
uniform vec2 screenSize;

uniform float waterHeight;
uniform float snowHeight;

const vec3 dirToLight = normalize(vec3(1, 3, 1));
const float FogDensity = 0.006;

//Calculates the visible surface color based on the Blinn-Phong illumination model
vec4 calculateLighting(vec4 materialColor, float specularIntensity, vec3 normalizedNormal, vec3 directionToViewer)
{
	vec4 color = materialColor;
	vec3 h = normalize(dirToLight + directionToViewer);
	color.xyz *= 0.9 * max(dot(normalizedNormal, dirToLight), 0) + 0.1;
	color.xyz += specularIntensity * pow(max(dot(h, normalizedNormal), 0), 50);
	return color;
}

vec4 getReflectionColor(vec3 normals) {
    // https://darrensweeney.net/2015/12/17/opengl-skybox-rendering/
    vec3 I = normalize(FRAG_position - cameraPos);
    vec3 R = reflect(I, normalize(normals));
    return texture(skybox, -R);
}

vec4 getBackgroundColor()
{
	return texture(background, gl_FragCoord.xy / screenSize);
}

float getFogFactor()
{
    float dist = gl_FragCoord.z / gl_FragCoord.w;
    float fogFactor = 1.0 /exp( (dist * FogDensity)* (dist * FogDensity));
    return clamp( fogFactor, 0.0, 1.0 );
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

vec4 terrainColor(vec3 dirToLight) {
    vec2 textureCoordinates = FRAG_position.xz * 10/255;
	vec3 n = normalize(FRAG_normals);

	mat3 tangentSpaceToViewSpace, viewSpaceToTangentSpace;
	calculateTangentSpace(n, FRAG_position, textureCoordinates, tangentSpaceToViewSpace, viewSpaceToTangentSpace);

	//material properties
    // Task 2.2.4 + 2.2.3
    // Based on: http://thedemonthrone.ca/projects/rendering-terrain/rendering-terrain-part-23-height-and-slope-based-colours/
    // Calculate terrain color
	float slope = acos(FRAG_normals.z);
    float blend = (length(slope) - 0.25f) * (1.0f / (0.5f - 0.25f));
    blend = clamp(blend, 0.0, 1.0);
    vec4 terrainColor = mix(texture(grassTexture, textureCoordinates), texture(rockTexture, textureCoordinates), blend);

    // Calculate alpha map
    vec3 alphaMapColor = texture(alphaMap, FRAG_position.xz / 255).xyz;
    // & use it to smootly Calculate road color
    vec4 roadColor = texture(roadColorTexture, textureCoordinates);
    roadColor = vec4((alphaMapColor * roadColor.xyz), roadColor.w);

    vec3 mapNormal = texture(roadNormalMap, textureCoordinates).xyz * 2 - 1;
    mapNormal.y *= -1;
    tangentSpaceToViewSpace[0] = normalize(tangentSpaceToViewSpace[0]);
    tangentSpaceToViewSpace[1] = normalize(tangentSpaceToViewSpace[1]);
    n = normalize(tangentSpaceToViewSpace * mapNormal);

    // Calculate road specular
    vec3 specularVector = vec3(texture(roadSpecularMap, textureCoordinates));
    float roadSpecular = specularVector.x;

    // Blend color, normals and specular together
    vec4 color = mix(terrainColor, roadColor, alphaMapColor.x);
    vec3 blendedNormals = mix(FRAG_normals, n, alphaMapColor.x);
    float blendedSpecular = alphaMapColor.x * roadSpecular;
    color = calculateLighting(color, blendedSpecular, blendedNormals, dirToLight);

    return color;
}

vec4 waterColor(vec3 dirToLight) {
    vec2 textureCoordinates = FRAG_position.xz * 10/255;
    vec2 animatedTextureCoordinates = (2 * textureCoordinates) + vec2(0.05 * animation, 0.05 * animation);
    float specular = 1.0;
    vec4 colorMaterial = 0.5 * texture(waterTexture, animatedTextureCoordinates);

    vec3 waterNormals = mix(texture(waterNormalMap, animatedTextureCoordinates).xyz, FRAG_normals, 0.5);

    vec4 colorReflection = mix(colorMaterial, getReflectionColor(waterNormals), 0.5);
    vec4 colorLighting = calculateLighting(colorReflection, specular, waterNormals, dirToLight);
    return colorLighting;
}

vec4 snowColor(vec3 dirToLight) {
    vec2 textureCoordinates = FRAG_position.xz * 0.01;
    float specular = 1.0;
    vec4 colorMaterial = texture(snowTexture, textureCoordinates);
    vec3 snowNormals = mix(texture(snowNormalMap, textureCoordinates).xyz, FRAG_normals, 0.5);
    vec4 colorReflection = mix(colorMaterial, getReflectionColor(snowNormals), 0.3);
    vec4 colorLighting = calculateLighting(colorReflection, specular, snowNormals, dirToLight);
    return colorLighting;
}

void main()
{
    // For Oren-Nayar lighting, uncomment the following:
    // Based on: https://stackoverflow.com/questions/40583715/oren-nayar-lighting-in-opengl-how-to-calculate-view-direction-in-fragment-shade#40596525
    vec3 dirToLight = vec3(1, 1, 0);
    // vec3 dirToLight = normalize(vec3(-(gl_FragCoord.xy - screenSize/2) / (screenSize/4), 1.0));
    // vec3 dirToLight = normalize(-fragmentPosition); //viewer is at the origin in camera space
    color = mix(waterColor(dirToLight), terrainColor(dirToLight), FRAG_waterFactor);
    color = mix(snowColor(dirToLight), color, FRAG_snowFactor);
    color = mix(getBackgroundColor(), color, getFogFactor());
}
