#include "StarForge.hpp"
#include "GlobalParameters.hpp"
#include <cvrConfig/ConfigManager.h>
#include <cvrKernel/PluginHelper.h>


using namespace cvr;

CVRPLUGIN(StarForge)

StarForge::StarForge() {
}

StarForge::~StarForge() {
	delete mUniverse;
}

bool StarForge::init() {

	std::cout << "Star Forge Init" << std::endl;
	mUniverse = new UniverseObject("Star Forge", true, false, false, true, true);

	PluginHelper::registerSceneObject(mUniverse, "Star Forge");
    mUniverse->attachToScene();
    mUniverse->setNavigationOn(true);
    mUniverse->setPosition(osg::Vec3(0, 0, 0));

    std::cout << "Star Forge Init finished" << std::endl;

	return true;
}

void StarForge::preFrame() {
}

void StarForge::postFrame() {
}

bool StarForge::processEvent(cvr::InteractionEvent * event) {

	return true;
}
