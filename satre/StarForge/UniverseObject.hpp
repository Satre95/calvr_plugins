#pragma once

#include <cvrKernel/SceneObject.h>
#include <cvrMenu/MenuButton.h>
#include <cvrConfig/ConfigManager.h>
#include "OSGPlanet.hpp"
#include <osg/Uniform>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>


class UniverseObject: public cvr::SceneObject
{
public:
    UniverseObject(std::string name, bool navigation, bool movable, bool clip, bool contextMenu, bool showBounds = false);
	virtual ~UniverseObject();
	
	virtual void menuCallback(cvr::MenuItem * item);

    void resetPosition();

protected:
    cvr::MenuButton * mResetPositionButton = nullptr;

    OSGPlanet * mPlanet = nullptr;
};