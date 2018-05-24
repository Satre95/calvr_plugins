#version 430 core 

layout(location = 0) in vec4 osg_Vertex;
// layout(location = 3) in vec4 osg_MultiTexCoord0;

uniform mat4 osg_ModelViewProjectionMatrix;

out vec3 TexCoords;

void main() {
	
	gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;
	TexCoords = osg_Vertex.xyz;
}