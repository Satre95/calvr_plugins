#version 430 core

layout (binding = 0) uniform samplerCube cubemap;


in vec3 TexCoords;

out vec4 OutColor;

void main() {

	OutColor = texture(cubemap, TexCoords);
	
}