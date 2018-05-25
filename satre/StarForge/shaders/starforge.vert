#version 430 core 
//---------------------------------------------------------
layout(location = 0) in vec4 osg_Vertex;
layout(location = 1) in vec3 osg_Normal;
layout(location = 2) in vec4 osg_Color;
layout(location = 3) in vec4 osg_MultiTexCoord0;
layout(location = 4) in vec4 osg_MultiTexCoord1;

//---------------------------------------------------------
uniform mat4 osg_ModelViewProjectionMatrix;
uniform mat4 osg_ModelViewMatrix;
uniform mat3 osg_NormalMatrix;
uniform mat4 osg_ViewMatrix; 
uniform mat4 osg_ViewMatrixInverse;

//---------------------------------------------------------
out vec4 FragColor;
out vec2 PosTexCoords;
out vec2 VelTexCoords;

//---------------------------------------------------------
vec3 ConvertCartesianToSpherical(vec3 cartCoords);

//---------------------------------------------------------
void main() {
	gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;
	
	FragColor = osg_Color;
	PosTexCoords = ConvertCartesianToSpherical(osg_Vertex.xyz).yz;
	VelTexCoords = osg_MultiTexCoord1.st;
}

//---------------------------------------------------------
/**
 \brief Converts the given cartesian coords to spherical.

 (x, y, z) -> (radius, inclination, azimuth)

 \note assumes 0 ≤ elevation ≤ π
 */
vec3 ConvertCartesianToSpherical(vec3 cartCoords) {
	float r = sqrt(
		cartCoords.x * cartCoords.x 
		+ cartCoords.y * cartCoords.y 
		+ cartCoords.z * cartCoords.z
		);
	return vec3(r,
					 acos(cartCoords.z / r),
					 atan(cartCoords.y / cartCoords.x)
	);
}