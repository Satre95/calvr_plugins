#include "UniverseObject.hpp"

#include <cvrKernel/NodeMask.h>
#include <cvrKernel/Navigation.h>
#include <cvrKernel/PluginHelper.h>
#include <osg/TexGen>
#include <osg/ShapeDrawable>

#include "GlobalParameters.hpp"

namespace params {
	glm::vec3 gPlanetCenter = glm::vec3(0.f);
	float gPlanetRadius = 100.f;
};

using namespace cvr;

UniverseObject::UniverseObject(std::string name, bool navigation, bool movable, bool clip, bool contextMenu, bool showBounds) :
        SceneObject(name,navigation,movable,clip,contextMenu,showBounds)
{
    setBoundsCalcMode(MANUAL);
    setBoundingBox(osg::BoundingBox(osg::Vec3(-1, -1, -1) * params::gPlanetRadius * 200.f, osg::Vec3(1, 1, 1) * params::gPlanetRadius * 200.f));

    if (contextMenu) {
        mScaleRangeSlider = new MenuRangeValue("Scale", 0.1, 100, 1.0);
        mScaleRangeSlider->setCallback(this);
        addMenuItem(mScaleRangeSlider);
        mFrameTimeItem = new cvr::MenuText("Last Frame timing (ms): ");
        addMenuItem(mFrameTimeItem);
    }


    int numRepulsors = ConfigManager::getInt("value", "Plugin.StarForge.NumRepulsors", 10);
    int numAttractors = ConfigManager::getInt("value", "Plugin.StarForge.NumAttractors", 10);
    std::cout << "Num Repulsors: " << numRepulsors << std::endl;
    std::cout << "Num Attractors: " << numAttractors << std::endl;

    std::string assetsPath = ConfigManager::getEntry("value", "Plugin.StarForge.AssetsPath",
                                                     "/home/satre/Developer/data/plugins/StarForge/");
    mPlanet = new OSGPlanet(numRepulsors, numAttractors, assetsPath);

    // Load and create the skybox
    mSkybox = new SkyBox;
    mSkybox->getOrCreateStateSet()->setTextureAttributeAndModes(0, new osg::TexGen);
    std::string skybox = assetsPath + ConfigManager::getEntry("value", "Plugin.StarForge.SkyboxPath",
                                                              "skyboxes/Belawor/"); // Relative to assets path

    mSkybox->setEnvironmentMap(0,
                               osgDB::readImageFile(skybox + "Left.tga"), osgDB::readImageFile(skybox + "Right.tga"),
                                osgDB::readImageFile(skybox + "Down.tga"), osgDB::readImageFile(skybox + "Up.tga"),
                               osgDB::readImageFile(skybox + "Front.tga"), osgDB::readImageFile(skybox + "Back.tga")
    );


    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    auto boundingBox = getOrComputeBoundingBox();
    geode->addDrawable(new osg::ShapeDrawable(
            new osg::Sphere(osg::Vec3(), boundingBox.radius())));
    geode->setCullingActive(false);
    mSkybox->addChild(geode);

//    addChild(mSkybox);
    addChild(mPlanet->GetGraph());
}

UniverseObject::~UniverseObject()
{
	delete mScaleRangeSlider;
	delete mPlanet;
    delete mFrameTimeItem;
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

void UniverseObject::PreFrame() {
    mPlanet->PreFrame();
}

void UniverseObject::PostFrame() {
    mPlanet->PostFrame();
    std::stringstream ss;
    ss << "Last Frame timing: " << PluginHelper::getLastFrameDuration() * 1000.f << " ms";
    mFrameTimeItem->setText(ss.str());
}