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
  vec4 fadeTint = vec4(smoothstep(vec3(0.f), vec3(1.f), vec3(u_time/ u_fadeTime)), 1.f);
  return fadeTint * colorIn;
}