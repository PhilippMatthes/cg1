#version 130

in vec4 position;
in vec4 normal;
in vec2 texCoords;

out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoords;

void main()
{
	TexCoords = texCoords;
	FragPos = position.xyz;
	Normal = normal.xyz;
}