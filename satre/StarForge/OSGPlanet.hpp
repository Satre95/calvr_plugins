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
#include <osg/AnimationPath>
#include <osg/PositionAttitudeTransform>

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
    osg::Group * InitParticleSystem(size_t numRepulsors, size_t numAttractors, std::string & assetsDir, bool drawSystem = false);
    osg::Group * InitPlanetDrawPipeline();

    osg::Program * LoadProgramForPhase(int phase);

    void SetupPhase1ColorsAndFades();
    void SetupPhase2ColorsAndFades();
    void SetupPhase3ColorsAndFades();


//    void CleanupPhase1(osg::Geode * geode);
//    void CleanupPhase2(osg::Geode * geode);
//    void CleanupPhase3(osg::Geode * geode);

    void UpdatePositionDataTexture();
    void UpdateColorDataTexture();
    void UpdateAgeVelDataTexture();

    osg::AnimationPath * CreateAnimationPhase1(float time);
    osg::AnimationPath * CreateAnimationPhase2(float time);
    osg::AnimationPath * CreateAnimationPhase3(float time);


	osgParticle::ParticleSystem * mSystem = nullptr;
	osgParticle::ModularEmitter * mParticleEmitter = nullptr;
	const std::string mAssetsDir;

	/// The root node of the planet scene graph
	osg::ref_ptr<osg::MatrixTransform> mRoot = nullptr;
    osg::ref_ptr<osg::MatrixTransform> mLastTransform = nullptr;
	osg::ref_ptr<osg::MatrixTransform> mScaleNode = nullptr; // Scale node
	osg::ref_ptr<osg::MatrixTransform> mRotationNode = nullptr;
	osg::ref_ptr<osg::PositionAttitudeTransform> mAnimationNode = nullptr;
//	osg::ref_ptr<osg::MatrixTransform> mTranslationNode = nullptr;

	osg::ref_ptr<osg::ShapeDrawable> mPlanetSphere = nullptr;
	osg::ref_ptr<osg::Geode> mPlanetGeode = nullptr;
	osg::Texture2D * mColorTexture = nullptr;

	// (x, y, z) = velocity, (w) = age
	osg::Texture2D * mAgeVelocityTexture = nullptr;
	osg::Texture2D * mPositionTexture = nullptr;

	osg::Uniform * mUGaussianSigma = nullptr;
	osg::Uniform * mUTime = nullptr;
	osg::Uniform * mUResolution = nullptr;

    float mParticleLifeTime = 10.f;
    float mRotationRate = 0.1f;

    osg::Program * mProgram1, *mProgram2, *mProgram3;

    bool mPhase2Switch = false, mPhase3Switch = false;
    float mPhase1Time, mPhase2Time, mPhase3Time;

};