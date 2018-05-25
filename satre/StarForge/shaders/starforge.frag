#version 430 core

//---------------------------------------------------------
layout (binding = 0) uniform sampler2D colorTexture;
layout (binding = 1) uniform sampler2D ageVelocityTexture;
uniform float maxParticleAge;

//---------------------------------------------------------
in VS_OUT {
	vec4 FragColor;
	vec2 ColorTexCoord;
	vec2 AgeVelTexCoord;
} fs_in;

//---------------------------------------------------------
out vec4 OutColor;

//---------------------------------------------------------s
void main() {
	vec4 texColor = texture(colorTexture, fs_in.ColorTexCoord);
	float age = texture(ageVelocityTexture, fs_in.AgeVelTexCoord).w / maxParticleAge;
	vec3 velocity = texture(ageVelocityTexture, fs_in.AgeVelTexCoord).xyz;

	OutColor = mix(texColor, fs_in.FragColor, 1.f - age);
}