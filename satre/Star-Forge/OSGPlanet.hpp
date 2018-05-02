#pragma once

#include <vector>

#include <osgParticle/ParticleSystem>
#include <osgParticle/ParticleSystemUpdater>

/// Wrapper class around an OSG Particle System
class OSGPlanet
{
public:
	OSGPlanet(size_t numRepulsors, size_t numAttractors, std::string & assetsDir);
	~OSGPlanet();

private:
	osgParticle::ParticleSystem * mSystem = nullptr;
	const std::string mAssetsDir;

	/// The root node of the particle system scene graph
	osg::Group * mRoot = nullptr;
};