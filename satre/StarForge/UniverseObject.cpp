#include "UniverseObject.hpp"

#include <cvrKernel/NodeMask.h>
#include <cvrKernel/Navigation.h>
#include <cvrKernel/PluginHelper.h>
#include <osg/TexGen>
#include <osgUtil/Optimizer>
#include <osg/AnimationPath>
#include <osgGA/AnimationPathManipulator>
#include <osgViewer/ViewerBase>

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
    setBoundingBox(osg::BoundingBox(osg::Vec3(-1, -1, -1) * params::gPlanetRadius * 400.f, osg::Vec3(1, 1, 1) * params::gPlanetRadius * 200.f));

    if (contextMenu) {
        mScaleRangeSlider = new MenuRangeValue("Scale", 0.1, 100, 1.0);
        mScaleRangeSlider->setCallback(this);
        addMenuItem(mScaleRangeSlider);

        mGaussianSigmaRangeSlider = new MenuRangeValue("Sigma", 1.f, 1000.f, 50.f);
        mGaussianSigmaRangeSlider->setCallback(this);
        addMenuItem(mGaussianSigmaRangeSlider);

        mRotationRateRangeSlider = new MenuRangeValue("Rotation", 0.f, 10.f, 0.1f);
        mRotationRateRangeSlider->setCallback(this);
        addMenuItem(mRotationRateRangeSlider);

        mFrameTimeItem = new cvr::MenuText("Last Frame timing (ms): ");
        addMenuItem(mFrameTimeItem);

        mNumParticlesItem = new cvr::MenuText("Number of Particles: ");
        addMenuItem(mNumParticlesItem);
    }


    int numRepulsors = ConfigManager::getInt("value", "Plugin.StarForge.NumRepulsors", 10);
    int numAttractors = ConfigManager::getInt("value", "Plugin.StarForge.NumAttractors", 10);
    std::cout << "Num Repulsors: " << numRepulsors << std::endl;
    std::cout << "Num Attractors: " << numAttractors << std::endl;

    std::string assetsPath = ConfigManager::getEntry("value", "Plugin.StarForge.AssetsPath",
                                                     "/home/satre/Developer/data/plugins/StarForge/");
    mPlanet = new OSGPlanet(numRepulsors, numAttractors, assetsPath);

    // Load and create the skybox
    mSkybox = new SkyBox(getOrComputeBoundingBox().radius());
    mSkybox->getOrCreateStateSet()->setTextureAttributeAndModes(0, new osg::TexGen);
    std::string skybox = assetsPath + ConfigManager::getEntry("value", "Plugin.StarForge.SkyboxPath",
                                                              "skyboxes/Belawor/"); // Relative to assets path

    mSkybox->setEnvironmentMap(0,
                               osgDB::readImageFile(skybox + "Left.tga"), osgDB::readImageFile(skybox + "Right.tga"),
                                osgDB::readImageFile(skybox + "Down.tga"), osgDB::readImageFile(skybox + "Up.tga"),
                               osgDB::readImageFile(skybox + "Front.tga"), osgDB::readImageFile(skybox + "Back.tga")
    );

    addChild(mSkybox);
    addChild(mPlanet->GetGraph());

    osgViewer::ViewerBase::Contexts contexts;
    cvr::CVRViewer::instance()->getContexts(contexts);
    if(contexts.empty() == false) {
        auto gl_state = contexts.front()->getState();
        gl_state->setUseModelViewAndProjectionUniforms(true);
        gl_state->setUseVertexAttributeAliasing(true);
    }

    PrepareCameraFlightPath();

    osgUtil::Optimizer optimizer;
    optimizer.optimize(mPlanet->GetGraph());
}

UniverseObject::~UniverseObject()
{
	delete mScaleRangeSlider;
	delete mGaussianSigmaRangeSlider;
	delete mRotationRateRangeSlider;
	delete mPlanet;
    delete mFrameTimeItem;
    delete mNumParticlesItem;
}

void UniverseObject::menuCallback(MenuItem * item)
{
    if(item == mScaleRangeSlider) {
	    setScale(mScaleRangeSlider->getValue());
	} else if(item == mGaussianSigmaRangeSlider) {
        mPlanet->SetShaderGaussianSigma(mGaussianSigmaRangeSlider->getValue());
    } else if(item == mRotationRateRangeSlider) {
        mPlanet->SetRotationRate(mRotationRateRangeSlider->getValue());
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
    {
        std::stringstream ss;
        ss << "Last Frame timing: " << PluginHelper::getLastFrameDuration() * 1000.f << " ms";
        mFrameTimeItem->setText(ss.str());
    }

    {
        std::stringstream ss;
        ss << "Number of particles: " << mPlanet->GetNumParticles();
        mNumParticlesItem->setText(ss.str());

    }
}
void UniverseObject::PrepareCameraFlightPath() {
    osg::AnimationPath * path = new osg::AnimationPath;
    path->setLoopMode(osg::AnimationPath::SWING);

    {
        osg::AnimationPath::ControlPoint cp;
        cp.setPosition(osg::Vec3d(0.0, -2.0 * params::gPlanetRadius, 0.0));
        path->insert(1.0, cp);
    }
    {
        osg::AnimationPath::ControlPoint cp;
        cp.setPosition(osg::Vec3d(-2.0 * params::gPlanetRadius, 0.0, 0.0));
        path->insert(5.0, cp);
    }
    {
        osg::AnimationPath::ControlPoint cp;
        cp.setPosition(osg::Vec3d(0.0, 0.0, -2.0 * params::gPlanetRadius));
        path->insert(9.0, cp);
    }

    osgGA::AnimationPathManipulator * apm = new osgGA::AnimationPathManipulator(path);
    cvr::CVRViewer::instance()->setCameraManipulator(apm);
    cvr::CVRViewer::instance()-
//    std::vector<osg::Camera*> cams;
//    cvr::CVRViewer::instance()->getCameras(cams);
//    std::cout << "Num cams: " << cams.size() << std::endl;

}