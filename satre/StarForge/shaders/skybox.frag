#version 430 core

layout (binding = 0) uniform samplerCube cubemap;

uniform float u_time;
uniform float u_fadeInDuration;
uniform float u_fadeOutDuration;
uniform float u_fadeOutTime; // The time point at which to begin fade out.

in vec3 TexCoords;

out vec4 OutColor;
void FadeIn(inout vec4 colorIn);
void FadeOut(inout vec4 colorIn);

void main() {

	OutColor = texture(cubemap, TexCoords);
	FadeIn(OutColor); FadeOut(OutColor);
}

void FadeIn(inout vec4 colorIn) {
	colorIn.xyz = mix(vec3(0.f), colorIn.xyz, min(u_time / u_fadeInDuration, 1.f));

}

void FadeOut(inout vec4 colorIn) {
	float t = max((u_time - u_fadeOutTime) / u_fadeOutDuration, 0.f);
	colorIn.xyz = mix(colorIn.xyz, vec3(0.f), t);
}
