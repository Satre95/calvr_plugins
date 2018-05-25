#version 430 core

//---------------------------------------------------------
layout (binding = 0) uniform sampler2D colorTexture;
layout (binding = 1) uniform sampler2D ageTexture;
uniform float maxParticleAge;

//---------------------------------------------------------
in vec4 FragColor;
in vec2 PosTexCoords;
in vec2 VelTexCoords;

//---------------------------------------------------------
out vec4 OutColor;

//---------------------------------------------------------s
void main() {
	vec4 texColor = texture(colorTexture, PosTexCoords);
	OutColor = texColor;
	
}