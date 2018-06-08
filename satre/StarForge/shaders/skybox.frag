#version 430 core

layout (binding = 0) uniform samplerCube cubemap;

uniform float u_time;
uniform float u_fadeTime;

in vec3 TexCoords;

out vec4 OutColor;
vec4 FadeInOut(vec4 colorIn);

void main() {

	OutColor = FadeInOut(texture(cubemap, TexCoords));
	
}

vec4 FadeInOut(vec4 colorIn) {
  vec4 fadeTint = smoothstep(vec4(0.f), vec4(1.f), vec4(u_time/ u_fadeTime));
  return fadeTint * colorIn;
}