#version 430 core

layout (binding = 0) uniform sampler2D posTexture;
layout (binding = 1) uniform sampler2D velTexture;

in vec4 FragColor;

out vec4 OutColor;

void main() {

	OutColor = FragColor;
	
}