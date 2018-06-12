#version 430 core

layout (binding = 0) uniform samplerCube cubemap;

uniform float u_time;
uniform float u_fadeInDuration;
uniform float u_fadeOutDuration;
uniform float u_fadeOutTime; // The time point at which to begin fade out.

in vec3 TexCoords;

out vec4 OutColor;
vec4 FadeIn(vec4 colorIn);
vec4 FadeOut(vec4 colorIn);

void main() {

	OutColor = FadeIn(texture(cubemap, TexCoords));
	OutColor = FadeOut(OutColor);
}

vec4 FadeIn(vec4 colorIn) {
	vec3 col = mix(vec3(0.f), colorIn.xyz, min(u_time / u_fadeInDuration, 1.f));
	return vec4(col, 1.f);
}

vec4 FadeOut(vec4 colorIn) {
	float t = max((u_time - u_fadeOutTime) / u_fadeOutDuration, 0.f);
	vec3 col = mix(colorIn.xyz, vec3(0.f), t);
	return vec4(col, 1.f);
}
