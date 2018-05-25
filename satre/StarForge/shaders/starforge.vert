#version 430 core 
#define M_PI 3.1415926535897932384626433832795

//---------------------------------------------------------
layout(location = 0) in vec4 osg_Vertex;
layout(location = 1) in vec3 osg_Normal;
layout(location = 2) in vec4 osg_Color;
// layout(location = 3) in vec4 osg_MultiTexCoord0;
// layout(location = 4) in vec4 osg_MultiTexCoord1;

//---------------------------------------------------------
uniform mat4 osg_ModelViewProjectionMatrix;
uniform mat4 osg_ModelViewMatrix;
uniform mat3 osg_NormalMatrix;
uniform mat4 osg_ViewMatrix; 
uniform mat4 osg_ViewMatrixInverse;

//---------------------------------------------------------
out VS_OUT {
	vec4 FragColor;
	vec2 ColorTexCoord;
	vec2 AgeVelTexCoord;
} vs_out;

//---------------------------------------------------------
vec3 ConvertCartesianToSpherical(vec3 cartCoords);
float MapToRange(float val, float inputMin, float inputMax, float outputMin, float outputMax);

//---------------------------------------------------------
void main() {
	gl_Position = osg_ModelViewProjectionMatrix * osg_Vertex;
	
	vs_out.FragColor = osg_Color;

	//Convert our position to spherical coordinates and map to texture coords
	vec3 sphereCoord = ConvertCartesianToSpherical(osg_Vertex.xyz);
	float inclination = sphereCoord.y;
	float azimuth = sphereCoord.z;
	float s = MapToRange(inclination, 0.f, M_PI, 0.f, 1.f);
    float t = MapToRange(azimuth, -M_PI, M_PI, 0.f, 1.f);
    vs_out.ColorTexCoord = vec2(s, t);
    vs_out.AgeVelTexCoord = vec2(s, t);
    
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

//---------------------------------------------------------
float MapToRange(float val, float inputMin, float inputMax, float outputMin, float outputMax) {
    return ((val - inputMin) / (inputMax - inputMin) * (outputMax - outputMin) + outputMin);
}