#version 430 core
#define NUM_STEPS 80
#define PI 3.1415926535897932384626433832795


//---------------------------------------------------------
layout (binding = 0) uniform sampler2D ColorTexture;
layout (binding = 1) uniform sampler2D AgeVelocityTexture;
layout (binding = 2) uniform sampler2D PositionTexture;

uniform float u_maxParticleAge;
uniform float u_gaussianSigma = 30.f;

//---------------------------------------------------------
in VS_OUT {
	vec4 FragColor;
	vec3 FragPos;
	vec2 ColorTexCoord;
	vec2 AgeVelTexCoord;
} fs_in;

//---------------------------------------------------------
out vec4 OutColor;

float Gaussian(float u, float sigma);

//---------------------------------------------------------s
void main() {
	vec3 localColor = vec3(0.f);
	float age = texture(AgeVelocityTexture, fs_in.AgeVelTexCoord).w / u_maxParticleAge;
	vec3 velocity = texture(AgeVelocityTexture, fs_in.AgeVelTexCoord).xyz;

	float s = 0.f, t = 0.f;
	float stepSize = 1.f / float(NUM_STEPS);
	for(int x = 0; x < NUM_STEPS; x++) {
		for(int y = 0; y < NUM_STEPS; y++) {
			vec3 texColor = texture(ColorTexture, vec2(s, t)).xyz;
			/// Particle position is same as normal on unit sphere.
			vec3 particlePos = texture(PositionTexture, vec2(s, t)).xyz;
			localColor += texColor * Gaussian(dot(particlePos, fs_in.FragPos) - 1, u_gaussianSigma);
			// localColor += texColor;
			t += stepSize;
		}

		s += stepSize;
	}

	vec3 finalColor = mix(localColor, texture(ColorTexture, fs_in.ColorTexCoord).xyz, 1.f - age);
	OutColor = vec4(finalColor, 1.f);
	// vec4 temp = texture(ColorTexture, fs_in.ColorTexCoord);
	// OutColor = mix(temp, fs_in.FragColor, age);
	// OutColor = vec4(abs(fs_in.FragPos), 1.f);
	// OutColor = texture(PositionTexture, fs_in.ColorTexCoord);
	
}

float Gaussian(float u, float sigma) {
	float term1 = inversesqrt(2.f * PI) * (1.f / sigma);
	float term2 = exp(-(u * u) / (2.f * sigma * sigma));
	return term1 * term2;
}
