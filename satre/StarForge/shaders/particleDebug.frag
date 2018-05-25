#version 430 core
layout (binding = 0) uniform sampler2D  spriteTexture;

in vec2 TexCoords;
in vec4 FragColor;

out vec4 OutColor;

void main() {
	vec4 color = texture(spriteTexture, TexCoords);
	if(length(color) == 0) {
		OutColor = FragColor;
	} else {
		OutColor = color * FragColor;
	}
	
}