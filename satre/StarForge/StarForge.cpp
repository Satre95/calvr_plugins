#include "StarForge.hpp"
#include "GlobalParameters.hpp"
#include <cvrConfig/ConfigManager.h>
#include <cvrKernel/PluginHelper.h>
#include <osg/Stats>
#include <sstream>

using namespace cvr;

CVRPLUGIN(StarForge)

StarForge::StarForge() {
}

StarForge::~StarForge() {
	delete mUniverse;
}

bool StarForge::init() {

	std::cout << "Star Forge Init" << std::endl;
	mUniverse = new UniverseObject("Star Forge", true, true, false, true, true);

	PluginHelper::registerSceneObject(mUniverse, "Star Forge");
    mUniverse->attachToScene();
    mUniverse->setNavigationOn(true);
    mUniverse->setMovable(true);
    mUniverse->setShowBounds(false);
    auto pos = ConfigManager::getVec3("x", "y", "z", "Plugin.StarForge.UniversePosition", osg::Vec3(0, -1000, 0));
    mUniverse->setPosition(pos);

    std::cout << "Star Forge Init finished" << std::endl;

	return true;
}

void StarForge::preFrame() {
    // Record the first time this method is called to offset for loading time.
    if(mFirstPreframeCall) {
        mFirstPreframeCall = false;
        mStartTime = cvr::PluginHelper::getProgramDuration();
        return;
    }
    auto elapsedTime = cvr::PluginHelper::getProgramDuration() - mStartTime;
    mUniverse->PreFrame(float(elapsedTime));
}

void StarForge::postFrame() {
    auto elapsedTime = cvr::PluginHelper::getProgramDuration() - mStartTime;
    mUniverse->PostFrame(float(elapsedTime));
}

bool StarForge::processEvent(cvr::InteractionEvent * event) {

	return true;
}