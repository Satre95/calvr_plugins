#ifndef PLANE_VEC_SHADERS_H
#define PLANE_VEC_SHADERS_H

#include <string>

std::string vecPlaneVertSrc =
"#version 150 compatibility \n"
"#extension GL_ARB_gpu_shader5 : enable \n"
"#extension GL_ARB_explicit_attrib_location : enable \n"
"\n"
"layout(location = 4) in vec3 value; \n"
"\n"
"uniform float min; \n"
"uniform float max; \n"
"\n"
"void main(void) \n"
"{ \n"
"    gl_FrontColor = gl_Color; \n"
"    gl_Position = gl_Vertex; \n"
"    gl_TexCoord[0].xyz = value; \n"
"    gl_TexCoord[0].w = clamp((length(value) - min) / (max - min), 0.0, 1.0); \n"
"} \n";

std::string vecPlaneGeomSrc = 
"#version 150 compatibility \n"
"#extension GL_EXT_geometry_shader4 : enable \n"
"\n"
"#define MAX_SEGMENTS_OUTPUT 100.0 \n"
"\n"
"uniform vec3 planePoint; \n"
"uniform vec3 planeNormal; \n"
"uniform vec3 planeUp; \n"
"uniform vec3 planeRight; \n"
"uniform mat3 planeBasisInv; \n"
"\n"
"out float colorLookup; \n"
"\n"
"void main() \n"
"{ \n"
"    int count = 0; \n"
"\n"
"    float dist[4]; \n"
"    for(int i = 0; i < 4; ++i) \n"
"    { \n"
"        dist[i] = dot(gl_PositionIn[i].xyz-planePoint,planeNormal); \n"
"        if(dist[i] > 0.0) \n"
"        { \n"
"            count++; \n"
"        } \n"
"    } \n"
"\n"
"    if(count == 0 || count == 4) \n"
"    { \n"
"        return; \n"
"    } \n"
"\n"
"    // project points to plane and express in terms of basis vectors \n"
"    vec2 basisPoints[4]; \n"
"    basisPoints[0] = (planeBasisInv * ((gl_PositionIn[0].xyz - (dist[0] * planeNormal)) - planePoint)).xy; \n"
"    basisPoints[1] = (planeBasisInv * ((gl_PositionIn[1].xyz - (dist[1] * planeNormal)) - planePoint)).xy; \n"
"    basisPoints[2] = (planeBasisInv * ((gl_PositionIn[2].xyz - (dist[2] * planeNormal)) - planePoint)).xy; \n"
"    basisPoints[3] = (planeBasisInv * ((gl_PositionIn[3].xyz - (dist[3] * planeNormal)) - planePoint)).xy; \n"
"\n"
"    // find min and max basis vector mults \n"
"    vec2 basisMin; \n"
"    vec2 basisMax; \n"
"\n"
"    basisMin = min(basisPoints[0],basisPoints[1]); \n"
"    basisMin = min(basisMin,basisPoints[2]); \n"
"    basisMin = min(basisMin,basisPoints[3]); \n"
"\n"
"    basisMax = max(basisPoints[0],basisPoints[1]); \n"
"    basisMax = max(basisMax,basisPoints[2]); \n"
"    basisMax = max(basisMax,basisPoints[3]); \n"
"\n"
"    basisMin = ceil(basisMin); \n"
"    basisMax = floor(basisMax); \n"
"\n"
"    // early out before inverse \n"
"    if(any(greaterThan(basisMin,basisMax))) \n"
"    { \n"
"	return; \n"
"    } \n"
"\n"
"    // create matrix to solve for barycentric coords \n"
"    mat3 tetMat = mat3(gl_PositionIn[0].xyz-gl_PositionIn[3].xyz,gl_PositionIn[1].xyz-gl_PositionIn[3].xyz,gl_PositionIn[2].xyz-gl_PositionIn[3].xyz); \n"
"    tetMat = inverse(tetMat); \n"
"\n"
"    float step; \n"
"    if((basisMax.x-basisMin.x) * (basisMax.y-basisMin.y) > MAX_SEGMENTS_OUTPUT) \n"
"    { \n"
"	step = max(floor(sqrt(((basisMax.x-basisMin.x) * (basisMax.y-basisMin.y)) / MAX_SEGMENTS_OUTPUT)),1.0); \n"
"    } \n"
"    else \n"
"    { \n"
"	step = 1.0; \n"
"    } \n"
"\n"
"    float vlen = length(planeUp) * 0.55; \n"
"\n"
"    // process each critical point on the plane \n"
"    for(float i = basisMin.x; i <= basisMax.x; i = i + step) \n"
"    { \n"
"	for(float j = basisMin.y; j <= basisMax.y; j = j + step) \n"
"	{ \n"
"	    vec4 coords; \n"
"	    coords.xyz = tetMat * (((i*planeUp+j*planeRight)+planePoint)-gl_PositionIn[3].xyz); \n"
"	    coords.w = 1.0 - coords.x - coords.y - coords.z; \n"
"\n"
"	    // see if point is out of tet \n"
"	    if(any(lessThan(coords,vec4(0.0,0.0,0.0,0.0))) || any(greaterThan(coords,vec4(1.0,1.0,1.0,1.0)))) \n"
"	    { \n"
"		continue; \n"
"	    } \n"
"\n"
"	    vec4 interp = coords.x * gl_TexCoordIn[0][0] + coords.y * gl_TexCoordIn[1][0] + coords.z * gl_TexCoordIn[2][0] + coords.w * gl_TexCoordIn[3][0]; \n"
"	    coords.xyz = (i*planeUp+j*planeRight)+planePoint; \n"
"	    gl_Position = gl_ModelViewProjectionMatrix * vec4(coords.xyz,1.0); \n"
"	    colorLookup = interp.w; \n"
"	    EmitVertex(); \n"
"	    interp.xyz = normalize(interp.xyz) * vlen; \n"
"	    gl_Position = gl_ModelViewProjectionMatrix * vec4(coords.xyz+interp.xyz,1.0); \n"
"	    colorLookup = interp.w; \n"
"	    EmitVertex(); \n"
"	    EndPrimitive(); \n"
"	} \n"
"    } \n"
"} \n";

std::string vecPlaneFragSrc = 
"#version 150 compatibility \n"
"\n"
"uniform sampler1D tex; \n"
"\n"
"in float colorLookup; \n"
"\n"
"void main() \n"
"{ \n"
"    gl_FragColor = texture1D(tex,colorLookup); \n"
"    //gl_FragColor = gl_Color * gl_TexCoord[0].x * gl_TexCoord[0].y; \n"
"    //gl_FragColor.rgb = vec3(gl_TexCoord[0].x,gl_TexCoord[0].x,gl_TexCoord[0].x); \n"
"    gl_FragColor.g = clamp(gl_FragColor.g+0.2,0.0,1.0); \n"
"    gl_FragColor.a = 1.0; \n"
"    //gl_FragColor = vec4(1.0,0.0,0.0,1.0); \n"
"} \n";

#endif
