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
	std::string gPluginConfigPrefix = "Plugin.StarForge.";
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

    osg::Vec3 universeTranslation = ConfigManager::getVec3(params::gPluginConfigPrefix + "UniversePosition");
    mUniverseTransform = new osg::MatrixTransform;
    osg::Matrix mat;
    mat.makeTranslate(universeTranslation);
    mUniverseTransform->setMatrix(mat);

    int numRepulsors = ConfigManager::getInt("value", params::gPluginConfigPrefix + "NumRepulsors", 10);
    int numAttractors = ConfigManager::getInt("value", params::gPluginConfigPrefix + "NumAttractors", 10);
    std::cout << "Num Repulsors: " << numRepulsors << std::endl;
    std::cout << "Num Attractors: " << numAttractors << std::endl;

    mAssetsPath = ConfigManager::getEntry("value", params::gPluginConfigPrefix + "AssetsPath",
                                                     "/home/satre/Developer/data/plugins/StarForge/");
    mPlanet = new OSGPlanet(numRepulsors, numAttractors, mAssetsPath);

//    mSkyboxes.reserve(3);
    mSkyboxes.push_back(new SkyBox(1, getOrComputeBoundingBox().radius()));
    mSkyboxes.push_back(new SkyBox(2, getOrComputeBoundingBox().radius()));
    mSkyboxes.push_back(new SkyBox(3, getOrComputeBoundingBox().radius()));

#pragma omp parallel for
    for (int i = 1; i <= 3; i++) {
        auto & skybox = mSkyboxes.at(i - 1);
        // Load and create the skyboxFiles
        skybox->getOrCreateStateSet()->setTextureAttributeAndModes(0, new osg::TexGen);
        std::string skyboxFiles = mAssetsPath + ConfigManager::getEntry("value",
                params::gPluginConfigPrefix + "Phase" + std::to_string(i) + ".SkyboxPath", "skyboxes/Belawor/");

        skybox->setEnvironmentMap(0,
                                  osgDB::readImageFile(skyboxFiles + "Left.tga"),
                                  osgDB::readImageFile(skyboxFiles + "Right.tga"),
                                  osgDB::readImageFile(skyboxFiles + "Down.tga"),
                                  osgDB::readImageFile(skyboxFiles + "Up.tga"),
                                  osgDB::readImageFile(skyboxFiles + "Front.tga"),
                                  osgDB::readImageFile(skyboxFiles + "Back.tga")
        );
    }

    addChild(mUniverseTransform);
    mCurrSkybox = mSkyboxes.at(0);
    mUniverseTransform->addChild(mCurrSkybox);
    mUniverseTransform->addChild(mPlanet->GetGraph());

    mPhase1Time = ConfigManager::getFloat(params::gPluginConfigPrefix + "Phase1.Fades.FadeInTime");
    mPhase2Time = ConfigManager::getFloat(params::gPluginConfigPrefix + "Phase2.Fades.FadeInTime");
    mPhase3Time = ConfigManager::getFloat(params::gPluginConfigPrefix + "Phase3.Fades.FadeInTime");
    std::cerr << mPhase1Time << std::endl;
    std::cerr << mPhase2Time << std::endl;
    std::cerr << mPhase3Time << std::endl;


    osgViewer::ViewerBase::Contexts contexts;
    cvr::CVRViewer::instance()->getContexts(contexts);
    if(contexts.empty() == false) {
        auto gl_state = contexts.front()->getState();
        gl_state->setUseModelViewAndProjectionUniforms(true);
        gl_state->setUseVertexAttributeAliasing(true);
    }

    SetupSound();
//    osgUtil::Optimizer optimizer;
//    optimizer.optimize(mPlanet->GetGraph());
}

UniverseObject::~UniverseObject()
{
	delete mScaleRangeSlider;
	delete mGaussianSigmaRangeSlider;
	delete mRotationRateRangeSlider;
	delete mPlanet;
    delete mFrameTimeItem;
    delete mNumParticlesItem;
    mAudioTrack.stop();

    if(!oasclient::ClientInterface::shutdown()) {
        std::cerr << "ERROR: Shutdown of connecation with OAS failed!" << std::endl;
    }
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

void UniverseObject::PreFrame(float runningTime) {
    mPlanet->PreFrame(runningTime);
    mCurrSkybox->PreFrame(runningTime);
}

void UniverseObject::PostFrame(float runningTime) {
    mPlanet->PostFrame(runningTime);
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

    if(runningTime >= mPhase2Time && !mPhase2Switch) {
        std::cout << "Skybox switching to phase 2" << std::endl;
        mUniverseTransform->removeChild(mCurrSkybox);
        mCurrSkybox = mSkyboxes.at(1);
        mUniverseTransform->addChild(mCurrSkybox);
        mPhase2Switch = true;
    } else if(runningTime >= mPhase3Time && !mPhase3Switch) {
        std::cout << "Skybox switching to phase 3" << std::endl;
        mUniverseTransform->removeChild(mCurrSkybox);
        mCurrSkybox = mSkyboxes.at(2);
        mUniverseTransform->addChild(mCurrSkybox);
        mPhase3Switch = true;
    }
}
/*
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
//    std::vector<osg::Camera*> cams;
//    cvr::CVRViewer::instance()->getCameras(cams);
//    std::cout << "Num cams: " << cams.size() << std::endl;

}
*/
void UniverseObject::SetupSound() {
   auto serverIP = ConfigManager::getEntry("ip", params::gPluginConfigPrefix + "OAS", "127.0.0.1");
   auto serverPort = ConfigManager::getInt("port", params::gPluginConfigPrefix + "OAS", 12345);
   auto soundFilepath = ConfigManager::getEntry("file", params::gPluginConfigPrefix + "AudioTrack", "sounds/AudioTrack.wav");
   auto gain = ConfigManager::getFloat("gain", params::gPluginConfigPrefix + "AudioTrack", 1.f);

   if(!oasclient::ClientInterface::initialize(serverIP, serverPort)) {
       std::cerr << "ERROR: UNable to connect to OAS server at IP: " << serverIP << " and port: " << serverPort << std::endl;
       return;
   }

   std::cerr << "Sound path: " << mAssetsPath + soundFilepath << std::endl;
   mAudioTrack.initialize(mAssetsPath + soundFilepath);
   if(!mAudioTrack.isValid()) {
       std::cerr << "ERROR: Unable to create sound specified by \'" << soundFilepath << "\'" << std::endl;
       return;
   }

   auto viewerPos = ConfigManager::getVec3("ViewerPosition", osg::Vec3(0.f, -500.f, 0.f));
   oasclient::Listener::getInstance().setPosition(viewerPos.x(), viewerPos.y(), viewerPos.z());

    // Set the listener's orientation so that it is looking down the positive Y axis,
    // and the "up" direction is in the positive Z axis. This means that the listener is
    // placed on the X-Y plane, with "up" being positive Z.
    oasclient::Listener::getInstance().setOrientation(0, 1, 0,   0, 0, 1);

    // Place the sound 3 units directly abobe of the listener
    mAudioTrack.setPosition(viewerPos.x(), viewerPos.y() + 1.f, viewerPos.z());
    mAudioTrack.setGain(gain);

    // No direction associated with the sound
    mAudioTrack.setDirection(0, 0, 0);
    mAudioTrack.setLoop(true);
    mAudioTrack.play();
}
