// This source code is property of the Computer Graphics and Visualization
// chair of the TU Dresden. Do not distribute! 
// Copyright (C) CGV TU Dresden - All Rights Reserved

#version 330

in vec4 position;
in vec2 offset;

out vec3 normals;
out vec4 vertexPosition;

uniform mat4 mvp;

//Returns the height of the procedural terrain at a given xz position
float getTerrainHeight(vec2 p);
vec3 calculateNormals(vec2 p);


void main()
{
    vec4 offsetPosition = vec4(position.x + offset.x, position.y, position.z + offset.y, position.w);

    normals = calculateNormals(offsetPosition.xz);

    vertexPosition = offsetPosition;

    float terrainHeight = getTerrainHeight(offsetPosition.xz);
    vec4 heightCorrectedPosition = vec4(offsetPosition.x, offsetPosition.y + terrainHeight, offsetPosition.z, offsetPosition.w);
	gl_Position = mvp * heightCorrectedPosition;
}

//source: https://gist.github.com/patriciogonzalezvivo/670c22f3966e662d2f83
float rand(vec2 c)
{
	return 2 * fract(sin(dot(c.xy ,vec2(12.9898,78.233))) * 43758.5453) - 1;
}

float perlinNoise(vec2 p )
{
	vec2 ij = floor(p);
	vec2 xy = p - ij;
	//xy = 3.*xy*xy-2.*xy*xy*xy;
	xy = .5*(1.-cos(3.1415926 * xy));
	float a = rand((ij+vec2(0.,0.)));
	float b = rand((ij+vec2(1.,0.)));
	float c = rand((ij+vec2(0.,1.)));
	float d = rand((ij+vec2(1.,1.)));
	float x1 = mix(a, b, xy.x);
	float x2 = mix(c, d, xy.x);
	return mix(x1, x2, xy.y);
}

//based on https://www.seedofandromeda.com/blogs/58-procedural-heightmap-terrain-generation
float getTerrainHeight(vec2 p)
{
	float total = 0.0;
	float maxAmplitude = 0.0;
	float amplitude = 1.0;
	float frequency = 0.02;
	for (int i = 0; i < 11; i++) 
	{
		total +=  ((1.0 - abs(perlinNoise(p * frequency))) * 2.0 - 1.0) * amplitude;
		frequency *= 2.0;
		maxAmplitude += amplitude;
		amplitude *= 0.45;
	}
	return 15 * total / maxAmplitude;
}

vec3 calculateNormals(vec2 p) {
    // Based on: https://stackoverflow.com/questions/13983189/opengl-how-to-calculate-normals-in-a-terrain-height-grid
    vec3 off = vec3(0.5, 0.5, 0.0);
    float hL = getTerrainHeight(p.xy - off.xz);
    float hR = getTerrainHeight(p.xy + off.xz);
    float hD = getTerrainHeight(p.xy - off.zy);
    float hU = getTerrainHeight(p.xy + off.zy);

    // deduce terrain normal
    vec3 N = vec3(0.0, 0.0, 0.0);
    N.x = hL - hR;
    N.y = hD - hU;
    N.z = 2.0;
    N = normalize(N);
    return N;
}