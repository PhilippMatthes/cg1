#version 130

in vec4 normalViewSpace;
in vec4 posViewSpace;
in vec4 vertexColor;
in vec2 vertexTexCoords;

out vec4 outColor;

uniform vec4 color;

uniform bool flatShading;
uniform bool perVertexColor;
uniform bool visualizeTexCoords;

void main()
{
	outColor = vec4(1, 0, 0, 1);
}