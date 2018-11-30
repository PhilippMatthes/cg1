#version 430

// Tessellation Evaluation Shader
// http://codeflow.org/entries/2010/nov/07/opengl-4-tessellation/

layout(quads, fractional_odd_spacing, cw) in;


uniform float perlinNoise1Frequency;
uniform float perlinNoise2Frequency;
uniform float perlinNoise1Height;
uniform float perlinNoise2Height;
uniform float waterHeight;


out vec3 FRAG_normals;
out vec4 FRAG_position;
out float FRAG_waterFactor;


uniform mat4 mvp;
uniform float animation;


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
	float amplitude1 = perlinNoise1Height;
	float amplitude2 = perlinNoise2Height;
	float frequency1 = perlinNoise1Frequency;
	float frequency2 = perlinNoise2Frequency;
	for (int i = 0; i < 11; i++)
	{
		total += ((1.0 - abs(perlinNoise(p * frequency1))) * 2.0 - 1.0) * amplitude1;
		total += ((1.0 - abs(perlinNoise(p * frequency2))) * 2.0 - 1.0) * amplitude2;
		frequency1 *= 2.0;
		frequency2 *= 2.0;
		maxAmplitude += max(amplitude1, amplitude2);
		amplitude1 *= 0.45;
		amplitude2 *= 0.45;
	}
	return max(waterHeight, 15 * total / maxAmplitude);
}


float getWaterHeight(vec2 p) {;
    return (0.2 * perlinNoise((p * 0.2) + vec2(animation, animation))) + waterHeight;
}


vec3 calculateNormals(vec2 p) {
    // Based on: https://stackoverflow.com/questions/13983189/opengl-how-to-calculate-normals-in-a-terrain-height-grid
    vec3 off = vec3(0.1, 0.1, 0.0);
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


void main(){
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;

    // Calculate mean position of the tessellated quad
    vec4 position =
    ((1-u) * (1-v) * gl_in[2].gl_Position +
    u * (1-v) * gl_in[0].gl_Position +
    u * v * gl_in[1].gl_Position +
    (1-u) * v * gl_in[3].gl_Position);

    FRAG_normals = calculateNormals(position.xz);

    FRAG_position = position;

    float terrainHeight = getTerrainHeight(position.xz);

    FRAG_waterFactor = clamp(terrainHeight - waterHeight, 0.0, 1.0);

    terrainHeight = mix(getWaterHeight(position.xy), terrainHeight, FRAG_waterFactor);

    vec4 heightCorrectedPosition = vec4(position.x, terrainHeight, position.y, position.w);

    gl_Position = mvp * heightCorrectedPosition;

}
