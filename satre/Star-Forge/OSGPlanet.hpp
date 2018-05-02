#pragma once

#include <vector>
#include <memory>

#include <osgParticle/ParticleSystem>
#include <osgParticle/ParticleSystemUpdater>

/// Wrapper class around an OSG Particle System
class OSGPlanet
{
public:
	OSGPlanet(size_t numRepulsors, size_t numAttractors, std::string & assetsDir);
	~OSGPlanet();

	osg::Group * GetGraph() const { return mRoot; }

private:
	osgParticle::ParticleSystem * mSystem = nullptr;
	const std::string mAssetsDir;

	/// The root node of the particle system scene graph
	osg::Group * mRoot = nullptr;
};