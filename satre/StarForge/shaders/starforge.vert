#version 450 

layout(location = 0) in vec4 osg_Vertex;
layout(location = 1) in vec3 osg_Normal;
layout(location = 2) in vec4 osg_Color;

//layout(location = 3)in vec4 osg_MultiTexCoord0;
// Can declare more tex coords


//uniform mat4 osg_ModelViewProjectionMatrix;
//uniform mat4 osg_ModelViewMatrix;
//uniform mat3 osg_NormalMatrix;
//uniform mat4 osg_ViewMatrix; 

uniform mat4 osg_ViewMatrix;
uniform mat4 osg_ProjectionMatrix;
uniform mat4 osg_ModelMatrix;
uniform mat4 osg_ViewMatrixInverse;


out vec4 FragColor;

void main() {
	
	gl_Position = osg_ProjectionMatrix * osg_ViewMatrix * osg_Vertex;
	//FragColor = osg_Color;
	FragColor = vec4(1.f, 0.f, 0.f, 1.f);
	
}