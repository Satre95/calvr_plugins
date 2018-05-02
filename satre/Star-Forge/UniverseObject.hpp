#pragma once

#include <cvrKernel/SceneObject.h>
#include <cvrMenu/MenuButton.h>
#include <cvrConfig/ConfigManager.h>
#include "OSGPlanet.hpp"


class UniverseObject: public cvr::SceneObject
{
public:
    UniverseObject(std::string name, bool navigation, bool movable, bool clip, bool contextMenu, bool showBounds = false);
	virtual ~UniverseObject();
	
	virtual void menuCallback(cvr::MenuItem * item);

    void resetPosition();

protected:
    cvr::MenuButton * _resetPositionButton;

    OSGPlanet * mPlanet;
};