#pragma once

#include <cvrKernel/SceneObject.h>
#include <cvrMenu/MenuButton.h>
#include <cvrConfig/ConfigManager.h>
#include <cvrMenu/MenuRangeValue.h>
#include "OSGPlanet.hpp"
#include <osg/Uniform>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <cvrMenu/MenuText.h>
#include <OAS/OASClient.h>
#include <vector>
#include "SkyBox.hpp"


class UniverseObject: public cvr::SceneObject
{
public:
    UniverseObject(std::string name, bool navigation, bool movable, bool clip, bool contextMenu, bool showBounds = false);
	virtual ~UniverseObject();
	
	virtual void menuCallback(cvr::MenuItem * item) override ;
    void setScale(float scale);

    void PreFrame(float runningTime);
    void PostFrame(float runningTime);

protected:

//    void PrepareCameraFlightPath();
    void SetupSound();

    cvr::MenuRangeValue * mScaleRangeSlider = nullptr;
    cvr::MenuRangeValue * mGaussianSigmaRangeSlider = nullptr;
    cvr::MenuRangeValue * mRotationRateRangeSlider = nullptr;
    cvr::MenuText * mFrameTimeItem = nullptr;
    cvr::MenuText * mNumParticlesItem = nullptr;
    float mScale = 1.0f;


    OSGPlanet * mPlanet = nullptr;
    std::vector<osg::ref_ptr<SkyBox >> mSkyboxes;
    SkyBox * mCurrSkybox;
    osg::MatrixTransform * mUniverseTransform = nullptr;
    oasclient::Sound mAudioTrack;
    std::string mAssetsPath;

    float mPhase1Time, mPhase2Time, mPhase3Time;
    bool mPhase2Switch = false, mPhase3Switch = false;
};