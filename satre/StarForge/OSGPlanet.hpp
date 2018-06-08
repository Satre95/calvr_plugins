#pragma once

#include <vector>
#include <memory>

#include <osgParticle/ParticleSystem>
#include <osgParticle/ParticleSystemUpdater>
#include <osg/MatrixTransform>
#include <osg/Shape>
#include <osg/ShapeDrawable>
#include <osg/Texture2D>
#include <osgParticle/ModularEmitter>

/// Wrapper class around an OSG Particle System
class OSGPlanet
{
public:
	OSGPlanet(size_t numRepulsors, size_t numAttractors, std::string & assetsDir);
	~OSGPlanet();
	void SetScale(osg::Matrix & mat) { mScaleNode->setMatrix(mat); }
	osg::Group * GetGraph() const { return mRoot; }

	void PreFrame(float runningTime);
	void PostFrame(float runningTime);

	int GetNumParticles() const { return mSystem->numParticles();}
	void SetShaderGaussianSigma(float sigma) {mUGaussianSigma->set(sigma); }
	void SetRotationRate(float rate) { mRotationRate = rate; }

private:
    void InitParticleSystem(size_t numRepulsors, size_t numAttractors, std::string & assetsDir, bool drawSystem = false);
    void InitPlanetDrawPipeline();

    void UpdatePositionDataTexture();
    void UpdateColorDataTexture();
    void UpdateAgeVelDataTexture();

	osgParticle::ParticleSystem * mSystem = nullptr;
	osgParticle::ModularEmitter * mParticleEmitter = nullptr;
	const std::string mAssetsDir;

	/// The root node of the particle system scene graph
	osg::ref_ptr<osg::MatrixTransform> mRoot = nullptr;
    osg::ref_ptr<osg::MatrixTransform> mLastTransform = nullptr;
	osg::ref_ptr<osg::MatrixTransform> mScaleNode = nullptr; // Scale node
	osg::ref_ptr<osg::MatrixTransform> mRotationNode = nullptr;
//	osg::ref_ptr<osg::MatrixTransform> mTranslationNode = nullptr;

	osg::ref_ptr<osg::ShapeDrawable> mPlanetSphere = nullptr;
	osg::Texture2D * mColorTexture = nullptr;

	// (x, y, z) = velocity, (w) = age
	osg::Texture2D * mAgeVelocityTexture = nullptr;
	osg::Texture2D * mPositionTexture = nullptr;

	osg::Uniform * mUGaussianSigma = nullptr;
	osg::Uniform * mUTime = nullptr;
	osg::Uniform * mUResolution = nullptr;

    float mParticleLifeTime = 10.f;
    float mRotationRate = 0.5f;
};