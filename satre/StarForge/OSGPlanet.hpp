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
	void SetScale(osg::Matrix & mat) { mRoot->setMatrix(mat); }
	osg::Group * GetGraph() const { return mRoot; }

	void PreFrame();
	void PostFrame();

private:
    void InitParticleSystem(size_t numRepulsors, size_t numAttractors, std::string & assetsDir);
    void InitPlanetGeometry();

	osgParticle::ParticleSystem * mSystem = nullptr;
	const std::string mAssetsDir;

	/// The root node of the particle system scene graph
	osg::ref_ptr<osg::MatrixTransform> mRoot = nullptr;
//	osg::ref_ptr<osg::MatrixTransform> mRotationNode = nullptr;
//	osg::ref_ptr<osg::MatrixTransform> mTranslationNode = nullptr;

    osg::ref_ptr<osg::Program> mPlanetDrawProgram = nullptr;

//    osg::ref_ptr<osg::Shader> mVertexShader = nullptr;
//    osg::ref_ptr<osg::Shader> mFragShader = nullptr;

    float mParticleLifeTime = 20;
    int mEstimatedMaxParticles = 0;
};