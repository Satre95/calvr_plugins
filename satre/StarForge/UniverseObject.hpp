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

#include "SkyBox.hpp"


class UniverseObject: public cvr::SceneObject
{
public:
    UniverseObject(std::string name, bool navigation, bool movable, bool clip, bool contextMenu, bool showBounds = false);
	virtual ~UniverseObject();
	
	virtual void menuCallback(cvr::MenuItem * item) override ;
    void setScale(float scale);

    void PreFrame();
    void PostFrame();

protected:
    cvr::MenuRangeValue * mScaleRangeSlider = nullptr;
    cvr::MenuText * mFrameTimeItem = nullptr;
    float mScale = 1.0f;


    OSGPlanet * mPlanet = nullptr;
    osg::ref_ptr<SkyBox> mSkybox = nullptr;
};