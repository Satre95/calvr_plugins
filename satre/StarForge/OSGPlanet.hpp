#pragma once

#include <vector>
#include <memory>

#include <osgParticle/ParticleSystem>
#include <osgParticle/ParticleSystemUpdater>
#include <osg/MatrixTransform>

/// Wrapper class around an OSG Particle System
class OSGPlanet
{
public:
	OSGPlanet(size_t numRepulsors, size_t numAttractors, std::string & assetsDir);
	~OSGPlanet();
	void SetScale(osg::Matrix & mat) { mScaleNode->setMatrix(mat); }
	osg::Group * GetGraph() const { return mRoot; }

private:
	osgParticle::ParticleSystem * mSystem = nullptr;
	const std::string mAssetsDir;

	/// The root node of the particle system scene graph
	osg::ref_ptr<osg::Group> mRoot = nullptr;
	osg::ref_ptr<osg::MatrixTransform> mScaleNode = nullptr;
//	osg::ref_ptr<osg::MatrixTransform> mRotationNode = nullptr;
//	osg::ref_ptr<osg::MatrixTransform> mTranslationNode = nullptr;
};