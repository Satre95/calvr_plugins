#pragma once
#include <vector>
#include <memory>
#include "Vorton.hpp"
#include "Tracer.hpp"

class Planet
{
public:
	Planet(float rad = 100.f);
	~Planet();

	void Update(const float timeStep);

	std::vector<VortonRef> & GetVortons() { return mVortons; }
	const std::vector<VortonRef> & GetVortons() const { return mVortons; }
	std::vector<Tracer> & GetTracers() { return mTracers; }
	const std::vector<Tracer> & GetTracers() const { return mTracers; }
private:
	std::vector<VortonRef> mVortons;
	std::vector<Tracer> mTracers;
	float mRadius;

	friend std::shared_ptr<Planet> CreatePlanet(float, size_t, size_t, size_t);
	friend std::shared_ptr<Planet> CreatePlanetWithCorotatingVortons(float radius, size_t numTracers);
	friend std::shared_ptr<Planet> CreatePlanetWithAttractorVortonRing(float radius, size_t numTracers);
	friend std::shared_ptr<Planet> CreatePlanetWithAttractorLatitudeRings(float radius, size_t numAttractorsPerLatitude, size_t numTracers);
};
typedef std::shared_ptr<Planet> PlanetRef;
extern PlanetRef CreatePlanet(float radius, size_t numRepulsors, size_t numAttractors, size_t numTracers);
extern PlanetRef CreatePlanetWithCorotatingVortons(float radius, size_t numTracers);
extern PlanetRef CreatePlanetWithAttractorVortonRing(float radius, size_t numTracers);
extern PlanetRef CreatePlanetWithAttractorLatitudeRings(float radius, size_t numAttractorsPerLatitude, size_t numTracers);