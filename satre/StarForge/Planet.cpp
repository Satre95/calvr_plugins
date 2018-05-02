#include "Planet.hpp"
#include "RepulsorVorton.hpp"
#include "AttractorVorton.hpp"
#include "math_helper.hpp"
#include <glm/gtx/norm.hpp>

Planet::Planet(float rad) : mRadius(rad)
{
}

Planet::~Planet()
{
	mVortons.clear();
}

void Planet::Update(const float timeStep) {
	//Update each tracer's force as influence by each Vorton
#pragma omp parallel for
	for (int i = 0; i < (int)mVortons.size(); i++)
	{
		for (size_t j = 0; j < mTracers.size(); j++)
		{
			VortonRef & vorton = mVortons.at(i);
			Tracer & tracer = mTracers.at(j);

			auto force = vorton->ComputeForceVector(tracer.mPosition);
			tracer.mForce += force;
		}
	}

	// For each tracer, update the acceleration, velocity, and position on the time step.
#pragma omp parallel for
	for (int i = 0; i < (int)mTracers.size(); i++)
	{
		Tracer & tracer = mTracers.at(i);
		auto accel = tracer.mForce / tracer.mMass;
		tracer.mVelocity = tracer.mVelocity + accel * timeStep;
		tracer.mPosition = tracer.mPosition + tracer.mVelocity * timeStep;

		// Correct the position, i.e. project it back onto the sphere.
		if (glm::length2(tracer.mPosition) > (mRadius * mRadius)) {
			// Find the target point on the surface of the sphere.
			glm::vec3 target = glm::normalize(tracer.mPosition) * mRadius;
			// Assign
			tracer.mPosition = target;
		}

		//Zero out the force vector for the next update
		tracer.mForce = glm::vec3(0.f);
	}

	// Update each vorton.
#pragma omp parallel for
	for (int i = 0; i < (int)mVortons.size(); i++) {
		mVortons.at(i)->Update(timeStep);
	}
}

PlanetRef CreatePlanet(float radius, size_t numRepulsors, size_t numAttractors, size_t numTracers) {
	auto planet = std::make_unique<Planet>(radius);

	// Allocate repulsors
	for (size_t i = 0; i < numRepulsors; i++) {
		auto point = RandomPointOnSphere() * radius;
		planet->mVortons.emplace_back(new RepulsorVorton(point));
		planet->mVortons.back()->SetVorticity(RandomFloat(30.f));
	}

	// Allocate attractors
	for (size_t i = 0; i < numAttractors; i++)
	{
		auto point = RandomPointOnSphere() * radius;
		planet->mVortons.emplace_back(new AttractorVorton(point));
		planet->mVortons.back()->SetVorticity(RandomFloat(30.f));
	}

	for (size_t i = 0; i < numTracers; i++)
	{
		auto point = RandomPointOnSphere() * radius;
		planet->mTracers.emplace_back(point);
	}
	return planet;
}

PlanetRef CreatePlanetWithCorotatingVortons(float radius, size_t numTracers) {
	PlanetRef planet = std::make_unique<Planet>(radius);

	//Place an attractors at equator
	auto vorticity = RandomFloat();
	glm::vec3 pos0 = ConvertSphericalToCartesian(glm::vec3(radius, 0.f, 0.f));
	planet->mVortons.emplace_back(new AttractorVorton(pos0));
	planet->mVortons.back()->SetVorticity(vorticity);
	glm::vec3 pos1 = ConvertSphericalToCartesian(glm::vec3(radius, 0.f, glm::pi<float>() / 16.f));
	planet->mVortons.emplace_back(new AttractorVorton(pos1));
	planet->mVortons.back()->SetVorticity(-vorticity);

	for (size_t i = 0; i < numTracers; i++)
	{
		auto point = RandomPointOnSphere() * radius;
		planet->mTracers.emplace_back(point);
	}

	return planet;
}

PlanetRef CreatePlanetWithAttractorVortonRing(float radius, size_t numTracers) {
	PlanetRef planet = std::make_unique<Planet>(radius);

	// Create a ring of corotating vortons at the equator.
	// Co-rotating means that all have the same direction of vorticity (CW or CCW)
	const size_t numAttractors = 8;
	const auto vorticity = RandomFloat(10.f);
	const float pi = glm::pi<float>();
	const float piBy16 = pi / 16.f;
	glm::vec3 pos;

	pos = ConvertSphericalToCartesian(glm::vec3(radius, 0.f, piBy16));
	planet->mVortons.emplace_back(new AttractorVorton(pos));
	planet->mVortons.back()->SetVorticity(vorticity);

	pos = ConvertSphericalToCartesian(glm::vec3(radius, piBy16, piBy16));
	planet->mVortons.emplace_back(new AttractorVorton(pos));
	planet->mVortons.back()->SetVorticity(-vorticity);

	pos = ConvertSphericalToCartesian(glm::vec3(radius, piBy16, 0.f));
	planet->mVortons.emplace_back(new AttractorVorton(pos));
	planet->mVortons.back()->SetVorticity(vorticity);

	pos = ConvertSphericalToCartesian(glm::vec3(radius, piBy16, -piBy16));
	planet->mVortons.emplace_back(new AttractorVorton(pos));
	planet->mVortons.back()->SetVorticity(-vorticity);

	pos = ConvertSphericalToCartesian(glm::vec3(radius, piBy16, -piBy16));
	planet->mVortons.emplace_back(new AttractorVorton(pos));
	planet->mVortons.back()->SetVorticity(vorticity);

	pos = ConvertSphericalToCartesian(glm::vec3(radius, 0.f, -piBy16));
	planet->mVortons.emplace_back(new AttractorVorton(pos));
	planet->mVortons.back()->SetVorticity(-vorticity);

	pos = ConvertSphericalToCartesian(glm::vec3(radius, -piBy16, -piBy16));
	planet->mVortons.emplace_back(new AttractorVorton(pos));
	planet->mVortons.back()->SetVorticity(vorticity);

	pos = ConvertSphericalToCartesian(glm::vec3(radius, -piBy16, 0.f));
	planet->mVortons.emplace_back(new AttractorVorton(pos));
	planet->mVortons.back()->SetVorticity(-vorticity);

	pos = ConvertSphericalToCartesian(glm::vec3(radius, -piBy16, piBy16));
	planet->mVortons.emplace_back(new AttractorVorton(pos));
	planet->mVortons.back()->SetVorticity(vorticity);

	auto zerothVorton = std::dynamic_pointer_cast<AttractorVorton>(planet->mVortons.front());
	for (size_t i = 1; i < numAttractors; i++) {
		auto aVorton = std::dynamic_pointer_cast<AttractorVorton>(planet->mVortons.at(i));
		aVorton->SetRotationAxis(zerothVorton->GetRotationAxis());
	}

	for (size_t i = 0; i < numTracers; i++)
	{
		auto point = RandomPointOnSphere() * radius;
		planet->mTracers.emplace_back(point);
	}

	return planet;
}
PlanetRef CreatePlanetWithAttractorLatitudeRings(float radius, size_t numAttractorsPerLatitude, size_t numTracers) {
	PlanetRef planet = std::make_shared<Planet>(radius);

	const float alpha = glm::pi<float>() / float(numAttractorsPerLatitude);
	const float beta = 2.f * glm::pi<float>() / float(numAttractorsPerLatitude);
	float flip = 1.f;
	const float vorticity = RandomFloat(20.f);

	float inclination = 0.f;
	while (inclination <= glm::pi<float>())
	{
		float azimuth = 0.f;
		while (azimuth <= 2.f * glm::pi<float>())
		{
			glm::vec3 pos = ConvertSphericalToCartesian(glm::vec3(radius, inclination, azimuth));
			planet->mVortons.emplace_back(new AttractorVorton(pos));
			planet->mVortons.back()->SetVorticity(flip * vorticity);
			azimuth += beta;
			flip *= -1;
		}
		inclination += alpha;
	}

	for (size_t i = 0; i < numTracers; i++)
	{
		auto point = RandomPointOnSphere() * radius;
		planet->mTracers.emplace_back(point);
	}

	return planet;
}