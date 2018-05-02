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
	mUniverse = new UniverseObject("Star Forge", false, false, false, true, false);

	PluginHelper::registerSceneObject(mUniverse);
    mUniverse->attachToScene();
    mUniverse->resetPosition();

	return true;
}

void StarForge::preFrame() {

}

void StarForge::postFrame() {

}

bool StarForge::processEvent(cvr::InteractionEvent * event) {

	return true;
}
