#pragma once
#include <glm/vec3.hpp>

class Tracer
{
public:
	Tracer(glm::vec3 pos = glm::vec3(1.f)) : mPosition(pos), mForce(0.f), mVelocity(0.f),
		mMass(1.f) {}
	~Tracer() {}

	glm::vec3 mPosition;
	glm::vec3 mForce;
	glm::vec3 mVelocity;
	float mMass;
};