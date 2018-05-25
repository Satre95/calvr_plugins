#version 430 core 

layout(location = 0) in vec4 osg_Vertex;
layout(location = 1) in vec3 osg_Normal;
layout(location = 2) in vec4 osg_Color;
layout(location = 3) in vec4 osg_MultiTexCoord0;


uniform mat4 osg_ModelViewProjectionMatrix;
uniform mat4 osg_ModelViewMatrix;
uniform mat3 osg_NormalMatrix;
uniform mat4 osg_ViewMatrix; 
uniform mat4 osg_ViewMatrixInverse;


out vec2 TexCoords;
out vec4 FragColor;

void main() {
	gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;
	
	FragColor = osg_Color;
	TexCoords = osg_MultiTexCoord0.st;
}