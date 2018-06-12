#version 430 core
#define NUM_STEPS 80
#define PI 3.1415926535897932384626433832795
#define MAX_NUM_COLORS 6

//---------------------------------------------------------
layout (binding = 0) uniform sampler2D ColorTexture;
layout (binding = 1) uniform sampler2D AgeVelocityTexture;
layout (binding = 2) uniform sampler2D PositionTexture;

uniform float u_maxParticleAge;
uniform float u_gaussianSigma = 30.f;
uniform vec2 u_resolution; // Viewport resolution in pixels
uniform float u_time;
uniform float u_fadeInTime = 0.f;
uniform float u_fadeInDuration;
uniform float u_fadeOutDuration;
uniform float u_fadeOutTime; // The time point at which to begin fade out.

uniform vec3 u_colors[MAX_NUM_COLORS];

//---------------------------------------------------------
in VS_OUT {
	vec4 FragColor;
	vec3 FragPos;
	vec2 ColorTexCoord;
	vec2 AgeVelTexCoord;
	} fs_in;

//---------------------------------------------------------
out vec4 OutColor;

//---------------------------------------------------------
const mat3 m = mat3( 0.80, 0.60, 0.4, 0.40, 0.80, 0.60, 0.60, 0.40, 0.80);

float Gaussian(float u, float sigma);
void FadeIn(inout vec4 colorIn);
void FadeOut(inout vec4 colorIn);
float fbm(vec3 p);
float fbm4(vec3 p);
float fbm6(vec3 p);
float fbm8(vec3 p);
float fbm10(vec3 p);
vec3 mod289(vec3 x);
vec4 mod289(vec4 x);
vec4 permute(vec4 x);
vec4 taylorInvSqrt(vec4 r);
float snoise(vec3 v);
vec3 doMagic(vec3 p);
vec3 pattern(in vec3 p, out vec3 q, out vec3 r);

//---------------------------------------------------------

void main() {

	float age = texture(AgeVelocityTexture, fs_in.AgeVelTexCoord).w / u_maxParticleAge;
	vec3 velocity = texture(AgeVelocityTexture, fs_in.AgeVelTexCoord).xyz;

	// vec3 p = abs(fs_in.FragPos);
	vec3 p = fs_in.FragPos;

	// float qLength = length(q);
	// vec3 incomingColor = texture(ColorTexture, fs_in.ColorTexCoord).xyz;

	// vec3 p = mix(incomingColor, q, age);

	vec3 q, r;
	vec3 pattern = pattern(p, q, r);
	vec3 texColor = texture(ColorTexture, fs_in.ColorTexCoord).xyz;

	vec3 color = mix(u_colors[0], u_colors[1], length(pattern));
	color = mix(color, u_colors[1], length(r));
	color = mix(color, u_colors[2], pattern.z) ;
	color = mix(color, u_colors[3], r.y);
	color = mix(color, u_colors[4], q.y);


	OutColor = vec4(color, 1.f);
	FadeIn(OutColor);
	FadeOut(OutColor);
}

float Gaussian(float u, float sigma) {
	float term1 = inversesqrt(2.f * PI) * (1.f / sigma);
	float term2 = exp(-(u * u) / (2.f * sigma * sigma));
	return term1 * term2;
}

void FadeIn(inout vec4 colorIn) {
	colorIn.xyz = mix(vec3(0.f), colorIn.xyz, min((u_time - u_fadeInTime) / u_fadeInDuration, 1.f));
}

void FadeOut(inout vec4 colorIn) {
	float t = max((u_time - u_fadeOutTime) / u_fadeOutDuration, 0.f);
	colorIn.xyz = mix(colorIn.xyz, vec3(0.f), t);
}

vec3 mod289(vec3 x) {
	return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 mod289(vec4 x) {
	return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 permute(vec4 x) {
	return mod289(((x*34.0)+1.0)*x);
}

vec4 taylorInvSqrt(vec4 r)
{
	return 1.79284291400159 - 0.85373472095314 * r;
}

float snoise(vec3 v)
{ 
	const vec2  C = vec2(1.0/6.0, 1.0/3.0) ;
	const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);

// First corner
vec3 i  = floor(v + dot(v, C.yyy) );
vec3 x0 =   v - i + dot(i, C.xxx) ;

// Other corners
vec3 g = step(x0.yzx, x0.xyz);
vec3 l = 1.0 - g;
vec3 i1 = min( g.xyz, l.zxy );
vec3 i2 = max( g.xyz, l.zxy );

	//   x0 = x0 - 0.0 + 0.0 * C.xxx;
	//   x1 = x0 - i1  + 1.0 * C.xxx;
	//   x2 = x0 - i2  + 2.0 * C.xxx;
	//   x3 = x0 - 1.0 + 3.0 * C.xxx;
	vec3 x1 = x0 - i1 + C.xxx;
	vec3 x2 = x0 - i2 + C.yyy; // 2.0*C.x = 1/3 = C.y
	vec3 x3 = x0 - D.yyy;      // -1.0+3.0*C.x = -0.5 = -D.y

// Permutations
i = mod289(i); 
vec4 p = permute( permute( permute( 
	i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
+ i.y + vec4(0.0, i1.y, i2.y, 1.0 )) 
+ i.x + vec4(0.0, i1.x, i2.x, 1.0 ));

// Gradients: 7x7 points over a square, mapped onto an octahedron.
// The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)
	float n_ = 0.142857142857; // 1.0/7.0
	vec3  ns = n_ * D.wyz - D.xzx;

	vec4 j = p - 49.0 * floor(p * ns.z * ns.z);  //  mod(p,7*7)

	vec4 x_ = floor(j * ns.z);
	vec4 y_ = floor(j - 7.0 * x_ );    // mod(j,N)

	vec4 x = x_ *ns.x + ns.yyyy;
	vec4 y = y_ *ns.x + ns.yyyy;
	vec4 h = 1.0 - abs(x) - abs(y);

	vec4 b0 = vec4( x.xy, y.xy );
	vec4 b1 = vec4( x.zw, y.zw );

	//vec4 s0 = vec4(lessThan(b0,0.0))*2.0 - 1.0;
	//vec4 s1 = vec4(lessThan(b1,0.0))*2.0 - 1.0;
	vec4 s0 = floor(b0)*2.0 + 1.0;
	vec4 s1 = floor(b1)*2.0 + 1.0;
	vec4 sh = -step(h, vec4(0.0));

	vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;
	vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;

	vec3 p0 = vec3(a0.xy,h.x);
	vec3 p1 = vec3(a0.zw,h.y);
	vec3 p2 = vec3(a1.xy,h.z);
	vec3 p3 = vec3(a1.zw,h.w);

//Normalise gradients
vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
p0 *= norm.x;
p1 *= norm.y;
p2 *= norm.z;
p3 *= norm.w;

// Mix final noise value
vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
m = m * m;
return 42.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1), 
	dot(p2,x2), dot(p3,x3) ) );
}

// float noise( in vec2 x )
// {
//  return sin(1.5*x.x)*sin(1.5*x.y);
// }
float fbm(vec3 p) {
	return 0.5f * snoise(p);
}

float fbm4( vec3 p )
{
	float f = 0.0;
	f += 0.5000*snoise( p ); p = m*p*2.02;
	f += 0.2500*snoise( p ); p = m*p*2.03;
	f += 0.1250*snoise( p ); p = m*p*2.01;
	f += 0.0625*snoise( p );
	return f/0.9375;
}

float fbm6( vec3 p )
{
	float f = 0.0;
	f += 0.500000*(0.5+0.5*snoise( p )); p = m*p*2.02;
	f += 0.250000*(0.5+0.5*snoise( p )); p = m*p*2.03;
	f += 0.125000*(0.5+0.5*snoise( p )); p = m*p*2.01;
	f += 0.062500*(0.5+0.5*snoise( p )); p = m*p*2.04;
	f += 0.031250*(0.5+0.5*snoise( p )); p = m*p*2.01;
	f += 0.015625*(0.5+0.5*snoise( p ));
	return f/0.96875;
}

float fbm8( vec3 p ) {
	float f = 0.0;
	f += 0.500000*(0.5+0.5*snoise( p )); p = m*p*2.02;
	f += 0.250000*(0.5+0.5*snoise( p )); p = m*p*2.03;
	f += 0.125000*(0.5+0.5*snoise( p )); p = m*p*2.01;
	f += 0.062500*(0.5+0.5*snoise( p )); p = m*p*2.04;
	f += 0.031250*(0.5+0.5*snoise( p )); p = m*p*2.01;
	f += 0.015625*(0.5+0.5*snoise( p ));
	f += 0.0078125*(0.5+0.5*snoise( p ));
	f += 0.00390625*(0.5+0.5*snoise( p ));
	return f/0.96875;	
}

float fbm10( vec3 p )
{
	float f = 0.0;
	f += 0.500000*(0.5+0.5*snoise( p )); p = m*p*2.02;
	f += 0.250000*(0.5+0.5*snoise( p )); p = m*p*2.03;
	f += 0.125000*(0.5+0.5*snoise( p )); p = m*p*2.01;
	f += 0.062500*(0.5+0.5*snoise( p )); p = m*p*2.04;
	f += 0.031250*(0.5+0.5*snoise( p )); p = m*p*2.01;
	f += 0.015625*(0.5+0.5*snoise( p ));
	f += 0.0078125*(0.5+0.5*snoise( p ));
	f += 0.003906250078125*(0.5+0.5*snoise( p ));
	f += 0.001953125003906250078125*(0.5+0.5*snoise( p ));
	f += 0.000976563*(0.5+0.5*snoise( p ));
	return f/0.96875;
}

float func( vec3 q, out vec4 ron )
{
	float ql = length( q );
	q.x += 0.05*sin(0.27*u_time+ql*4.1);
	q.y += 0.05*sin(0.23*u_time+ql*4.3);
	q.z += 0.05*sin(0.23*u_time+ql*4.3);

	q *= 0.5;

	vec3 o = vec3(0.0);
	o.x = 0.5 + 0.5*fbm4( vec3(2.0*q          )  );
	o.y = 0.5 + 0.5*fbm4( vec3(2.0*q+vec3(5.2))  );
	o.z = 0.5 + 0.5*fbm4( vec3(2.0*q+vec3(5.2))  );

	float ol = length( o );
	o.x += 0.02*sin(0.12*u_time+ol)/ol;
	o.y += 0.02*sin(0.14*u_time+ol)/ol;
	o.z += 0.02*sin(0.16*u_time+ol)/ol;

	vec3 n;
	n.x = fbm6( vec3(4.0*o+vec3(9.2))  );
	n.y = fbm6( vec3(4.0*o+vec3(5.7))  );
	n.z = fbm6( vec3(4.0*o+vec3(5.7))  );

	vec3 p = 4.0*q + 4.0*n;

	float f = 0.5 + 0.5*fbm4( p );

	f = mix( f, f*f*f*3.5, f*abs(n.x) );

	float g = 0.5 + 0.5*sin(4.0*p.x)*sin(4.0*p.y);
	f *= 1.0-0.5*pow( g, 8.0 );

	ron = vec4( o, n );
	
	return f;
}

vec3 doMagic(vec3 p) {
	vec3 q = p*0.6;

	vec4 on = vec4(0.0);
	float f = func(q, on);

	vec3 col = vec3(0.0);
	col = mix( vec3(0.2,0.1,0.4), vec3(0.3,0.05,0.05), f );
	col = mix( col, vec3(0.9,0.9,0.9), dot(on.zw,on.zw) );
	col = mix( col, vec3(0.4,0.3,0.3), 0.5*on.y*on.y );
	col = mix( col, vec3(0.0,0.2,0.4), 0.5*smoothstep(1.2,1.3,abs(on.z)+abs(on.w)) );
	col = clamp( col*f*2.0, 0.0, 1.0 );

	vec3 nor = normalize( vec3( dFdx(f)*u_resolution.x, 6.0, dFdy(f)*u_resolution.y) );

	vec3 lig = normalize( vec3( 0.9, -0.2, -0.4 ) );
	float dif = clamp( 0.3+0.7*dot( nor, lig ), 0.0, 1.0 );
	vec3 bdrf;
	bdrf  = vec3(0.70,0.90,0.95)*(nor.y*0.5+0.5);
	bdrf += vec3(0.15,0.10,0.05)*dif;
	col *= 1.2*bdrf;
	col = 1.0-col;
	return 1.1*col*col;
}

vec3 pattern(in vec3 p, out vec3 q, out vec3 r) {
	float pLength = length(p);
	p.x += 0.15*sin(0.35 * u_time + pLength * 4.1);
	p.y += 0.15*cos(0.40 * u_time + pLength * 4.7);
	p.x += 0.15*sin(0.25 * u_time + pLength * 4.5);

	q = vec3( fbm(p + vec3(0, 0, 0)),
		fbm(p + vec3(5.2f, 1.3f, 0.4f)), 
		fbm(p + vec3(3.2f, 1.6f, 2.4f))
		);

	r = vec3( fbm6(p + 4.f  * q + vec3(0, 0, 0)),
		fbm6(p + 4.f * q + vec3(1.7f, 9.2f, 9.4f)), 
		fbm6(p + 4.f * q + vec3(8.3f, 2.8f, 5.4f))
		);

	return vec3(sin(fbm(p + 4.f * r)));
}