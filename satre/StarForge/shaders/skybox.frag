#version 430 core

layout (binding = 0) uniform samplerCube cubemap;

uniform float u_time;
struct FadeInfo {
	float fadeInTime;
	float fadeInDuration;
	float fadeOutTime;
	float fadeOutDuration;
};

uniform FadeInfo u_phaseFadeInfo;

in vec3 TexCoords;

out vec4 OutColor;
void FadeIn(inout vec4 colorIn, in float startTime, in float duration);
void FadeOut(inout vec4 colorIn, in float startTime, in float duration);
void ProcessFade(inout vec4 colorIn);

void main() {
	OutColor = texture(cubemap, TexCoords);
	ProcessFade(OutColor);
}

void ProcessFade(inout vec4 colorIn) {
	FadeIn(colorIn, u_phaseFadeInfo.fadeInTime, u_phaseFadeInfo.fadeInDuration);
	FadeOut(colorIn, u_phaseFadeInfo.fadeOutTime, u_phaseFadeInfo.fadeOutDuration);
}

void FadeIn(inout vec4 colorIn, in float startTime, in float duration) {
	float t = clamp((u_time - startTime) / duration, 0.f, 1.f);
	colorIn.xyz = mix(vec3(0.f), colorIn.xyz, t);
}

void FadeOut(inout vec4 colorIn, in float startTime, in float duration) {
	float t = clamp((u_time - startTime) / duration, 0.f, 1.f);
	colorIn.xyz = mix(colorIn.xyz, vec3(0.f), t);
}
