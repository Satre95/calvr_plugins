#include "UniverseObject.hpp"

#include <cvrKernel/NodeMask.h>
#include <cvrKernel/Navigation.h>
#include <cvrKernel/PluginHelper.h>
#include "GlobalParameters.hpp"

namespace params {
	glm::vec3 gPlanetCenter = glm::vec3(0.f);
	float gPlanetRadius = 100.f;
};

using namespace cvr;

UniverseObject::UniverseObject(std::string name, bool navigation, bool movable, bool clip, bool contextMenu, bool showBounds) : SceneObject(name,navigation,movable,clip,contextMenu,showBounds) {
	setBoundsCalcMode(MANUAL);
	setBoundingBox(osg::BoundingBox(osg::Vec3(-100000,-100000,-100000),osg::Vec3(100000,100000,100000)));

	mResetPositionButton = NULL;
	if(contextMenu) {
		mResetPositionButton = new MenuButton("Reset Position");
		mResetPositionButton->setCallback(this);
		addMenuItem(mResetPositionButton);
	}

	int numRepulsors = ConfigManager::getInt("value", "Plugin.StarForge.NumRepulsors", 10);
	int numAttractors = ConfigManager::getInt("value", "Plugin.StarForge.NumAttractors", 10);
	std::string assetsPath = ConfigManager::getEntry("value", "Plugin.StarForge.AssetsPath", "/home/satre/Developer/data/plugins/StarForge");
	mPlanet = new OSGPlanet(numRepulsors, numAttractors, assetsPath);

	addChild(mPlanet->GetGraph());
}

UniverseObject::~UniverseObject()
{
	delete mResetPositionButton;
	delete mPlanet;
}

void UniverseObject::menuCallback(MenuItem * item)
{
	if(item == mResetPositionButton) {
		resetPosition();
		return;
	}

	SceneObject::menuCallback(item);
}

void UniverseObject::resetPosition()
{
	setNavigationOn(false);
	osg::Matrix m, ms, mt;
	m.makeRotate((90.0/180.0)*M_PI,osg::Vec3(1.0,0,0));
	ms.makeScale(osg::Vec3(1000.0,1000.0,1000.0));
	mt.makeTranslate(osg::Vec3(0,0,-Navigation::instance()->getFloorOffset()));
	setTransform(m*ms*mt);
	setNavigationOn(true);
}