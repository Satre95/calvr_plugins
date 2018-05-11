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

	if(contextMenu) {
		mScaleRangeSlider = new MenuRangeValue("Scale", 0.1, 100, 1.0);
		mScaleRangeSlider->setCallback(this);
		addMenuItem(mScaleRangeSlider);
	}


	int numRepulsors = ConfigManager::getInt("value", "Plugin.StarForge.NumRepulsors", 10);
	int numAttractors = ConfigManager::getInt("value", "Plugin.StarForge.NumAttractors", 10);
	std::cout << "Num Repulsors: " << numRepulsors << std::endl;
	std::cout << "Num Attractors: " << numAttractors << std::endl;

	std::string assetsPath = ConfigManager::getEntry("value", "Plugin.StarForge.AssetsPath", "/home/satre/Developer/data/plugins/StarForge");
	mPlanet = new OSGPlanet(numRepulsors, numAttractors, assetsPath);

    /*
    osg::Program * pgm1 = new osg::Program;
    pgm1->setName( "StarForgeShader" );
    std::string shaderPath = ConfigManager::getEntry("value", "Plugin.StarForge.ShadersPath", "/home/satre/CVRPlugins/satre/StarForge/shaders");
    std::cout << shaderPath << std::endl;
    pgm1->addShader(osg::Shader::readShaderFile(osg::Shader::VERTEX, shaderPath + "starforge.vert"));
    pgm1->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, shaderPath + "starforge.frag"));
    */
	addChild(mPlanet->GetGraph());
}

UniverseObject::~UniverseObject()
{
	delete mScaleRangeSlider;
	delete mPlanet;
}

void UniverseObject::menuCallback(MenuItem * item)
{
    if(item == mScaleRangeSlider) {
	    setScale(mScaleRangeSlider->getValue());
	}

	SceneObject::menuCallback(item);
}

void UniverseObject::setScale(float scale) {
    mScale = scale;
    // Set the scale matrix
    osg::Matrix m;
    m.makeScale(osg::Vec3(mScale, mScale, mScale));
    mPlanet->SetScale(m);

}