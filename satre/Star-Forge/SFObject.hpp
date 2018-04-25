#pragma once

#include <cvrKernel/SceneObject.h>
#include <cvrMenu/MenuButton.h>


class SFObject: public cvr::SceneObject
{
public:
    SFObject(std::string name, bool navigation, bool movable, bool clip, bool contextMenu, bool showBounds = false);
	virtual ~SFObject();
	
	virtual void menuCallback(cvr::MenuItem * item);

    void resetPosition();

protected:
    cvr::MenuButton * _resetPositionButton;
};