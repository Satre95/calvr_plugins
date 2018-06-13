#include "OSGPlanet.hpp"
#include "GlobalParameters.hpp"
#include "SphereTangentShooter.hpp"
#include "AttractorVorton.hpp"
#include "RepulsorVorton.hpp"
#include "math_helper.hpp"
#include "SpherePlacer.hpp"
#include "PositionCorrectionOperator.hpp"
#include <osg/Geometry>
#include <osg/Geode>
#include <osg/Point>
#include <osgParticle/ModularEmitter>
#include <osgParticle/ConstantRateCounter>
#include <osgParticle/ModularProgram>
#include <osgDB/FileUtils>
#include <osg/Program>
#include <osg/io_utils>
#include <osg/Matrix>
#include <osgParticle/LinearInterpolator>
#include <osg/BlendFunc>
#include <osg/PositionAttitudeTransform>
#include <unordered_map>
#include <algorithm>

#include <cvrConfig/ConfigManager.h>
#include <cvrKernel/PluginHelper.h>
#include <cvrKernel/CVRViewer.h>
#include <cstring>
#include <mutex>
#include <osg/Depth>
#include <osg/Material>

// These correspond to the macros defined in each respective shader.
#define NUM_COLORS_PHASE_1 4
#define NUM_COLORS_PHASE_2 5
#define NUM_COLORS_PHASE_3 4
#define MAX_NUM_COLORS() std::max(std::max(NUM_COLORS_PHASE_1, NUM_COLORS_PHASE_2), NUM_COLORS_PHASE_3)


using namespace cvr;

//int getestimatedMaxNumberOfParticles(osgParticle::ConstantRateCounter * counter, double lifetime);
osg::Image * CreateImage(int width, int height, int numComponents);
osg::Texture2D * CreateTexture(int width, int height, int numComponents);
void ClearImage(osg::Image * image);
std::pair<float, float> GetTextureCoordsofParticle(osgParticle::Particle * particle);

OSGPlanet::OSGPlanet(size_t numRepulsors, size_t numAttractors, std::string & assetsDir) : mAssetsDir(assetsDir){
    mRotationNode = new osg::MatrixTransform;
    mScaleNode = new osg::MatrixTransform;
    mRotationNode->addChild(mScaleNode);
    mRoot = mRotationNode;
    mLastTransform = mScaleNode;

    auto partSystemRoot = InitParticleSystem(numRepulsors, numAttractors, assetsDir, false);
    auto planetRoot = InitPlanetDrawPipeline();

    // Get the phase 1 animation
    auto animPath1 = CreateAnimationPhase1();
    // Create a node to move the things around.
    auto xForm = new osg::PositionAttitudeTransform;
    xForm->setUpdateCallback(new osg::AnimationPathCallback(animPath1));
    mLastTransform->addChild(xForm);

    xForm->addChild(planetRoot);
    xForm->addChild(partSystemRoot);
    mPhase1Time = ConfigManager::getFloat(params::gPluginConfigPrefix + "Phase1.Fades.FadeInTime");
    mPhase2Time = ConfigManager::getFloat(params::gPluginConfigPrefix + "Phase2.Fades.FadeInTime");
    mPhase3Time = ConfigManager::getFloat(params::gPluginConfigPrefix + "Phase3.Fades.FadeInTime");


}

OSGPlanet::~OSGPlanet() = default;

osg::Group* OSGPlanet::InitParticleSystem(size_t numRepulsors, size_t numAttractors, std::string & assetsDir, bool drawSystem) {

    auto shadersPath = cvr::ConfigManager::getEntry("value", params::gPluginConfigPrefix + "ShadersPath", "/home/satre/CVRPlugins/satre/StarForge/shaders/");
    auto systemRoot = new osg::Group;

    /// Init the particle system
    mSystem = new osgParticle::ParticleSystem;
    std::string assetsPath = cvr::ConfigManager::getEntry("value", params::gPluginConfigPrefix + "AssetsPath", "/home/satre/Assets/StarForge/");
    mSystem->setDefaultAttributes(assetsPath + "particle.png", false, false);

    // Init a template particle, which all future particles will be copies of.
    osgParticle::Particle pTemplate;
    pTemplate.setLifeTime(mParticleLifeTime);
    pTemplate.setMass(0.001f); // 1 gram of mass
    pTemplate.setSizeRange(osgParticle::rangef(2.f, 2.f));
    pTemplate.setColorRange(osgParticle::rangev4(osg::Vec4(1.f, 0.5f, 0.3f, 1.f), osg::Vec4(0.5f, 0.7f, 1.0f, 1.f)));
    mSystem->setDefaultParticleTemplate(pTemplate);
    // Init the mParticleEmitter.
    mParticleEmitter = new osgParticle::ModularEmitter;
    mParticleEmitter->setParticleSystem(mSystem);

    // Init the counter for the mParticleEmitter
//    auto * counter = new osgParticle::RandomRateCounter;
//    counter->setRateRange(80, 120);
    auto * counter = new osgParticle::ConstantRateCounter;
    counter->setMinimumNumberOfParticlesToCreate(25);
    counter->setNumberOfParticlesPerSecondToCreate(50);
    std::cerr << "Estimated max number of particles: " << counter->getEstimatedMaxNumOfParticles(pTemplate.getLifeTime()) << std::endl;
    mParticleEmitter->setCounter(counter);

    // Init the placer for the mParticleEmitter
    osg::Vec3 center = GLM2OSG(params::gPlanetCenter);

    auto * placer = new SpherePlacer(center, params::gPlanetRadius);
    mParticleEmitter->setPlacer(placer);

    // Init the shooter for the mParticleEmitter
    auto * shooter = new SphereTangentShooter;
    mParticleEmitter->setShooter(shooter);

    // Add the shooter to the scene graph.
    systemRoot->addChild(mParticleEmitter);

    // Create a program to control the post-spawning behavior of the particles
    auto * program = new osgParticle::ModularProgram;
    program->setParticleSystem(mSystem);
    program->addOperator(new PositionCorrectionOperator);
    // Add the program to the scene graph
    systemRoot->addChild(program);

    osg::ref_ptr<osg::Vec3Array> vortonsVertices = new osg::Vec3Array;
    osg::ref_ptr<osg::Vec4Array> vortonsColors = new osg::Vec4Array;
	for (size_t i = 0; i < (numAttractors + numRepulsors); i++) {
        auto pos = RandomPointOnSphere() * params::gPlanetRadius;
        Vorton *v;
        if (i < numAttractors) {
            v = new AttractorVorton(pos);
        } else {
            v = new RepulsorVorton(pos);
        }
        v->SetVorticity(RandomFloat(8.f));
        program->addOperator(v);

        if (drawSystem) {
            vortonsVertices->push_back(GLM2OSG(v->GetPosition()));
            osg::Vec4 color;
            if (i < numAttractors) {
                color = osg::Vec4(0.f, 1.f, 0.f, 1.f);
            } else {
                color = osg::Vec4(1.f, 0.f, 0.f, 1.f);
            }
            vortonsColors->push_back(color);
        }
    }

    if(drawSystem) {
        auto normals = new osg::Vec3Array;
        normals->push_back(osg::Vec3(0.f, -1.f, 0.f));

        auto vortonsGeom = new osg::Geometry;
        vortonsGeom->setVertexArray(vortonsVertices);
        vortonsGeom->setColorArray(vortonsColors, osg::Array::BIND_PER_VERTEX);
        vortonsGeom->setNormalArray(normals, osg::Array::BIND_OVERALL);
        vortonsGeom->addPrimitiveSet(new osg::DrawArrays(GL_POINTS, 0, numAttractors + numRepulsors));
        vortonsGeom->getOrCreateStateSet()->setAttribute(new osg::Point(12.f), osg::StateAttribute::ON);
        auto vortonsGeode = new osg::Geode;
        vortonsGeode->setCullingActive(false);
        vortonsGeode->addDrawable(vortonsGeom);
        // Create a shader program to render vortons
        auto drawProgram = new osg::Program;
        auto vertexShader = osg::Shader::readShaderFile(osg::Shader::VERTEX,
                                                        osgDB::findDataFile(shadersPath + "particleDebug.vert"));
        auto fragShader = osg::Shader::readShaderFile(osg::Shader::FRAGMENT,
                                                      osgDB::findDataFile(shadersPath + "particleDebug.frag"));
        drawProgram->addShader(vertexShader);
        drawProgram->addShader(fragShader);
        vortonsGeode->getOrCreateStateSet()->setAttribute(drawProgram,
                                                          osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
        systemRoot->addChild(vortonsGeode);

        // Create a drawable target for the particle system
        auto geode = new osg::Geode;
        geode->setCullingActive(false);
        geode->addDrawable(mSystem);
        // Recycle the vortons geode stateset for particle debug draw.
        geode->setStateSet(vortonsGeode->getOrCreateStateSet());
        systemRoot->addChild(geode);
    }

    // Create a particle system updater
    auto psUpdater = new osgParticle::ParticleSystemUpdater;
    psUpdater->addParticleSystem(mSystem);

    systemRoot->addChild(psUpdater);
    return systemRoot;
}

osg::Group* OSGPlanet::InitPlanetDrawPipeline() {
    auto planetRoot = new osg::Group;

    // Create the sphere
    mPlanetSphere = new osg::ShapeDrawable(
            new osg::Sphere(GLM2OSG(params::gPlanetCenter), params::gPlanetRadius));

    auto colorArray = new osg::Vec4Array;
    colorArray->push_back(osg::Vec4(0.3f, 0.3f, 0.76f, 0.3f));
    mPlanetSphere->setColorArray(colorArray, osg::Array::BIND_OVERALL);
    mPlanetSphere->setUseVertexBufferObjects(true);
    mPlanetSphere->setUseVertexArrayObject(true);
    mPlanetSphere->setUseDisplayList(false);

    // Create the Geode (Geometry Node)
    auto geode = new osg::Geode;
    geode->addDrawable(mPlanetSphere);
    mPlanetGeode = geode;

    // Setup the textures that will hold the particle data
    auto stateset = geode->getOrCreateStateSet();
    stateset->setAttributeAndModes( new osg::Depth(osg::Depth::LESS));
    auto * counter = dynamic_cast<osgParticle::ConstantRateCounter*>( mParticleEmitter->getCounter());

    auto uMaxParticleAge = new osg::Uniform(osg::Uniform::Type::FLOAT, "u_maxParticleAge");
    stateset->addUniform(uMaxParticleAge);
    uMaxParticleAge->set(float(mSystem->getDefaultParticleTemplate().getLifeTime()));

    mUGaussianSigma = new osg::Uniform(osg::Uniform::Type::FLOAT, "u_gaussianSigma");
    stateset->addUniform(mUGaussianSigma);
    mUGaussianSigma->set(50.f);

    mUResolution = new osg::Uniform(osg::Uniform::Type::FLOAT_VEC2, "u_resolution");
    stateset->addUniform(mUResolution);
    osg::Vec2 dims(cvr::PluginHelper::getScreenInfo(0)->width, cvr::PluginHelper::getScreenInfo(0)->height);
    mUResolution->set(dims);

    mUTime = new osg::Uniform(osg::Uniform::Type::FLOAT, "u_time");
    stateset->addUniform(mUTime);
    mUTime->set(0.f);

    int texSize = int(std::ceil(std::sqrt(counter->getEstimatedMaxNumOfParticles(mParticleLifeTime)))) + 1; // Err on the side of caution
    std::cerr << "Initializing textures with width " << texSize << std::endl;
    {
        mColorTexture = CreateTexture(texSize, texSize, 4);
        auto image = CreateImage(texSize, texSize, 4);
        mColorTexture->setImage(image);
        mColorTexture->setUnRefImageDataAfterApply(false);
        stateset->setTextureAttributeAndModes(0, mColorTexture);
    }

    {
        mAgeVelocityTexture = CreateTexture(texSize, texSize, 4);
        auto image = CreateImage(texSize, texSize, 4);
        mAgeVelocityTexture->setImage(image);
        mAgeVelocityTexture->setUnRefImageDataAfterApply(false);
        stateset->setTextureAttributeAndModes(1, mAgeVelocityTexture);
    }

    {
        mPositionTexture = CreateTexture(texSize, texSize, 4);
        auto image = CreateImage(texSize, texSize, 4);
        mPositionTexture->setUnRefImageDataAfterApply(false);
        mPositionTexture->setImage(image);
        stateset->setTextureAttributeAndModes(2, mPositionTexture);
    }
    stateset->setAttributeAndModes(new osg::BlendFunc);
    stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

    // Setup the colors uniform
    auto uni = new osg::Uniform(osg::Uniform::Type::FLOAT_VEC3, "u_colors", MAX_NUM_COLORS());
    stateset->addUniform(uni);
    // Preload all the programs for each phase
    mProgram1 = LoadProgramForPhase(1);
    mProgram2 = LoadProgramForPhase(2);
    mProgram3 = LoadProgramForPhase(3);

    SetupPhase1ColorsAndFades();
    stateset->setAttribute(mProgram1);
    stateset->setDataVariance(osg::Object::DataVariance::DYNAMIC);

    // Add it to the scene graph
    planetRoot->addChild(geode);
    return planetRoot;
}


void OSGPlanet::PreFrame(float runningTime) {
    auto rotation = std::fmod(mRotationRate * runningTime, 2.0 * osg::PI);
    osg::Matrix rotMat;
    rotMat.makeRotate(rotation, osg::Vec3(0.f, 0.f, 1.f));
    mRotationNode->setMatrix(rotMat);

    mUTime->set(float(runningTime));

//    UpdatePositionDataTexture();
    UpdateColorDataTexture();
//    UpdateAgeVelDataTexture();
}


void OSGPlanet::PostFrame(float runningTime) {
    if(runningTime >= mPhase2Time && !mPhase2Switch) {
        std::cerr << "Planet switching to phase 2" << std::endl;
        SetupPhase2ColorsAndFades();
        mPlanetGeode->getOrCreateStateSet()->removeAttribute(osg::StateAttribute::Type::PROGRAM);
        mPlanetGeode->getOrCreateStateSet()->setAttributeAndModes(mProgram2);
        mPhase2Switch = true;
    } else if(runningTime >= mPhase3Time && !mPhase3Switch) {
        std::cerr << "Planet switching to phase 3" << std::endl;
        SetupPhase3ColorsAndFades();
        mPlanetGeode->getOrCreateStateSet()->removeAttribute(osg::StateAttribute::Type::PROGRAM);
        mPlanetGeode->getOrCreateStateSet()->setAttributeAndModes(mProgram3);
        mPhase3Switch = true;
    }
}

void OSGPlanet::UpdatePositionDataTexture() {
    auto image = mPositionTexture->getImage();
    auto texWidth = image->s();
    ClearImage(image);

    if (!image->isDataContiguous()) std::cerr << "Image data is not contiguous!" << std::endl;
    auto data = reinterpret_cast<float *>(image->data());

    // One slot for each particle.
    int numParticles = mSystem->numParticles();
#pragma omp parallel for
    for (int i = 0; i < numParticles; ++i) {
        auto particle = mSystem->getParticle(i);
        auto & pos = particle->getPosition();
        auto texCoords = GetTextureCoordsofParticle(particle);
        auto & s = texCoords.first;
        auto & t = texCoords.second;
        // Convert from texture coordinates to image coordinates
        auto x = int(std::floor(s * image->s()));
        auto y = int(std::floor(t * image->t()));
        int index = (x * image->t() + y) * 4;

        data[index] = pos.x(); data[index+1] = pos.y(), data[index+2] = pos.z(); data[index + 4] = 1.f;
    }
    image->dirty();
}

void OSGPlanet::UpdateColorDataTexture() {
    auto image = mColorTexture->getImage();
    auto texWidth = image->s();
    ClearImage(image);

    if (!image->isDataContiguous()) std::cerr << "Image data is not contiguous!" << std::endl;
    auto data = reinterpret_cast<float *>(image->data());

    // Every frame, refill the textures with the particle data.
    // assume column major: x * height + y
    int numParticles = mSystem->numParticles();
#pragma omp parallel for
    for (int i = 0; i < numParticles; ++i) {
        // Get a particle and convert its pos into spherical coords.
        auto particle = mSystem->getParticle(i);
        auto texCoords = GetTextureCoordsofParticle(particle);
        auto & s = texCoords.first;
        auto & t = texCoords.second;

        // Convert from texture coordinates to image coordinates
        auto x = int(std::floor(s * image->s()));
        auto y = int(std::floor(t * image->t()));

        int index = (x * image->t() + y) * 4;
        data[index] += particle->getCurrentColor().r();
        data[index+1] += particle->getCurrentColor().g();
        data[index+2] += particle->getCurrentColor().b();
        data[index+3] += 1.f;
    }

    // Average out color data
#pragma omp parallel for
    for (int y = 0; y < image->t(); ++y) {
        for (int x = 0; x < image->s(); ++x) {
            int i = (x * image->t() + y) * 4;
            if(data[i+3] != 0.f) {
                float & numContribs = data[i+3];
                data[i] /= numContribs; data[i+1] /= numContribs;
                data[i+2] /= numContribs;
            }
        }
    }

    // mark for upload
    image->dirty();
}

void OSGPlanet::UpdateAgeVelDataTexture() {
    auto image = mAgeVelocityTexture->getImage();
    auto texWidth = image->s();
    ClearImage(image);
    if (!image->isDataContiguous()) std::cerr << "Image data is not contiguous!" << std::endl;
    auto data = reinterpret_cast<float *>(image->data());

    std::unordered_map<std::pair<int, int>, float> contribs;

    int numParticles = mSystem->numParticles();

    std::mutex contribsMutex;
#pragma omp parallel for
    for (int i = 0; i < numParticles; ++i) {
        // Get a particle and convert its pos into spherical coords.
        auto particle = mSystem->getParticle(i);
        auto & vel = particle->getVelocity();
        double age = particle->getAge();
        auto texCoords = GetTextureCoordsofParticle(particle);
        auto & s = texCoords.first;
        auto & t = texCoords.second;

        auto x = int(std::floor(s * image->s()));
        auto y = int(std::floor(t * image->t()));

        int index = (x * image->t() + y) * 4;

        data[index] = vel.x();
        data[index + 1] = vel.y();
        data[index + 2] = vel.z();
        data[index + 3] = float(age);

        {
            std::lock_guard<std::mutex> lock(contribsMutex);
            if (contribs.find(std::make_pair(x, y)) == contribs.end())
                contribs.insert(std::make_pair(std::make_pair(x, y), 1.f));
            else
                contribs.at(std::make_pair(x, y)) += 1.f;
        }
    }

    // Average out contributions
#pragma omp parallel for
    for (int y = 0; y < image->t(); ++y) {
        for (int x = 0; x < image->s(); ++x) {
            if(contribs.find(std::make_pair(x, y)) != contribs.end()) {
                int i = (x * image->t() + y) * 4;
                auto & numContribs = contribs.at(std::make_pair(x, y));
                data[i] /= numContribs;
                data[i+1] /= numContribs;
                data[i+2] /= numContribs;
                data[i+3] /= numContribs;
            }
        }
    }

    image->dirty();
}

osg::Program * OSGPlanet::LoadProgramForPhase(int phase) {
    // Load the shaders
    auto shadersPath = cvr::ConfigManager::getEntry("value", params::gPluginConfigPrefix + "ShadersPath", "/home/satre/CVRPlugins/satre/StarForge/shaders/");
    auto vertexShader = osg::Shader::readShaderFile(osg::Shader::VERTEX,
            osgDB::findDataFile(shadersPath + "starforge_phase" + std::to_string(phase) + ".vert"));
    auto fragShader = osg::Shader::readShaderFile(osg::Shader::FRAGMENT,
            osgDB::findDataFile(shadersPath + "starforge_phase" + std::to_string(phase) + ".frag"));

    if(!vertexShader) {
        std::cerr << "ERROR: Unable to load vertex shader in " << shadersPath << std::endl;
        return nullptr;
    }
    if(!fragShader) {
        std::cerr << "ERROR: Unable to load fragment shader in " << shadersPath << std::endl;
        return nullptr;
    }

    // Setup the programmable pipeline
    auto drawProgram = new osg::Program;
    drawProgram->addShader(vertexShader);
    drawProgram->addShader(fragShader);

    return drawProgram;
}

void OSGPlanet::SetupPhase1ColorsAndFades() {
    auto stateset = mPlanetGeode->getOrCreateStateSet();
    // Setup the colors for this phase
    auto uni = stateset->getUniform("u_colors");
    auto numColors = ConfigManager::getInt("value", params::gPluginConfigPrefix + "Phase1.Colors.NumColors", 0);
    for (unsigned int i = 1; i <= numColors; ++i) {
        auto color = ConfigManager::getVec3(params::gPluginConfigPrefix + "Phase1.Colors.Color" + std::to_string(i));
        uni->setElement(i - 1, color);
    }

    // Setup the fade in and out times for this phase
    float fadeInDuration = cvr::ConfigManager::getFloat("value", params::gPluginConfigPrefix + "Phase1.Fades.FadeInDuration");
    auto uFadeInDuration = new osg::Uniform(osg::Uniform::Type::FLOAT, "u_fadeInDuration");
    stateset->addUniform(uFadeInDuration);
    uFadeInDuration->set(fadeInDuration);

    float fadeOutTime = ConfigManager::getFloat("value", params::gPluginConfigPrefix + "Phase1.Fades.FadeOutTime", 42.f);
    auto uFadeOutTime = new osg::Uniform(osg::Uniform::Type::FLOAT, "u_fadeOutTime");
    stateset->addUniform(uFadeOutTime);
    uFadeOutTime->set(fadeOutTime);

    float fadeOutDuration = ConfigManager::getFloat("value", params::gPluginConfigPrefix + "Phase1.Fades.FadeOutDuration", 3.f);
    auto uFadeOutDuration = new osg::Uniform(osg::Uniform::Type::FLOAT, "u_fadeOutDuration");
    stateset->addUniform(uFadeOutDuration);
    uFadeOutDuration->set(fadeOutDuration);

}

void OSGPlanet::SetupPhase2ColorsAndFades() {
    auto stateset = mPlanetGeode->getOrCreateStateSet();
    // Setup the colors for this phase
    auto uni = stateset->getUniform("u_colors");
    auto numColors = ConfigManager::getInt("value", params::gPluginConfigPrefix + "Phase2.Colors.NumColors", 0);
    for (unsigned int i = 1; i <= numColors; ++i) {
        osg::Vec3 color = ConfigManager::getVec3(params::gPluginConfigPrefix + "Phase2.Colors.Color" + std::to_string(i));
        uni->setElement(i - 1, color);
    }

    // Setup the fade in and out times for this phase
    float fadeInDuration = cvr::ConfigManager::getFloat("value", params::gPluginConfigPrefix + "Phase2.Fades.FadeInDuration");
    auto uFadeInDuration = stateset->getUniform("u_fadeInDuration");
    uFadeInDuration->set(fadeInDuration);

    float fadeOutTime = ConfigManager::getFloat("value", params::gPluginConfigPrefix + "Phase2.Fades.FadeOutTime", 42.f);
    auto uFadeOutTime = stateset->getUniform("u_fadeOutTime");
    uFadeOutTime->set(fadeOutTime);

    float fadeOutDuration = ConfigManager::getFloat("value", params::gPluginConfigPrefix + "Phase2.Fades.FadeOutDuration", 3.f);
    auto uFadeOutDuration = stateset->getUniform("u_fadeOutDuration");
    uFadeOutDuration->set(fadeOutDuration);
}

void OSGPlanet::SetupPhase3ColorsAndFades() {
    auto stateset = mPlanetGeode->getOrCreateStateSet();
    // Setup the colors for this phase
    auto uni = stateset->getUniform("u_colors");
    auto numColors = ConfigManager::getInt("value", params::gPluginConfigPrefix + "Phase3.Colors.NumColors", 0);
    std::cout << "Setting " << numColors << " colors for phase 3" << std::endl;
    for (unsigned  int i = 1; i <= numColors; ++i) {
        auto color = ConfigManager::getVec3(params::gPluginConfigPrefix + "Phase3.Colors.Color" + std::to_string(i));
        uni->setElement(i - 1, color);
    }

    // Setup the fade in and out times for this phase
    float fadeInDuration = cvr::ConfigManager::getFloat("value", params::gPluginConfigPrefix + "Phase3.Fades.FadeInDuration");
    auto uFadeInDuration = stateset->getUniform("u_fadeInDuration");
    uFadeInDuration->set(fadeInDuration);

    float fadeOutTime = ConfigManager::getFloat("value", params::gPluginConfigPrefix + "Phase3.Fades.FadeOutTime", 42.f);
    auto uFadeOutTime = stateset->getUniform("u_fadeOutTime");
    uFadeOutTime->set(fadeOutTime);

    float fadeOutDuration = ConfigManager::getFloat("value", params::gPluginConfigPrefix + "Phase3.Fades.FadeOutDuration", 3.f);
    auto uFadeOutDuration = stateset->getUniform("u_fadeOutDuration");
    uFadeOutDuration->set(fadeOutDuration);
}

/**
 * Planet starts far away and we gradually get closer. Once it fills our vision, ...
 * @return
 */
osg::AnimationPath * OSGPlanet::CreateAnimationPhase1() {
    auto path = new osg::AnimationPath;
    path->setLoopMode(osg::AnimationPath::NO_LOOPING);

    auto numPoints = cvr::ConfigManager::getInt("value", params::gPluginConfigPrefix + "Phase1.AnimationPath.NumPoints", 0);
    for (int i = 1; i <= numPoints; ++i) {
        std::string tag = params::gPluginConfigPrefix + "Phase1.AnimationPath.Point" + std::to_string(i);
        auto point = cvr::ConfigManager::getVec4(tag);
        osg::AnimationPath::ControlPoint cp;
        cp.setPosition(osg::Vec3d(point.x(), point.y(), point.z()));
        path->insert(point.w(), cp);
    }

    return path;
}

osg::AnimationPath * OSGPlanet::CreateAnimationPhase2() {}

osg::AnimationPath * OSGPlanet::CreateAnimationPhase3() {}

//int getestimatedMaxNumberOfParticles(osgParticle::ConstantRateCounter * counter, double lifetime) {
//    int minNumParticles =  static_cast<int>(counter->getMinimumNumberOfParticlesToCreate() * 60.0f * lifetime);
//    int baseNumPartciles = static_cast<int>(counter->getNumberOfParticlesPerSecondToCreate() * lifetime);
//    return osg::maximum(minNumParticles, baseNumPartciles);
//}

osg::Image * CreateImage(int width, int height, int numComponents) {
    GLenum pixelFormat;
    switch (numComponents) {
        case 1: pixelFormat = GL_R; break;
        case 2: pixelFormat = GL_RG; break;
        case 3: pixelFormat = GL_RGB; break;
        default: pixelFormat = GL_RGBA; break;
    }

    auto image = new osg::Image;
    image->allocateImage(width, width, 1, pixelFormat, GL_FLOAT);
    auto data = reinterpret_cast<float *>(image->data());
    // zero fill image
#pragma omp parallel for
    for (int y = 0; y < image->t(); ++y) {
        for (int x = 0; x < image->s(); ++x) {
            int i = (x * image->t() + y) * 4;
            for (int j = 0; j < numComponents; ++j) {
                data[i + j] = 0.f;
            }
        }
    }

    return image;
}

osg::Texture2D * CreateTexture(int width, int height, int numComponents) {
    GLint  internalFormat;
    switch (numComponents) {
        case 1: internalFormat = GL_R32F;
            break;

        case 2: internalFormat = GL_RG32F;
            break;

        case 3: internalFormat = GL_RGB32F;
            break;

        default:internalFormat = GL_RGBA32F;
            break;
    }
    auto tex = new osg::Texture2D;
    tex->setResizeNonPowerOfTwoHint(false);
    tex->setDataVariance(osg::Object::DYNAMIC);
    tex->setTextureSize(width, height);
    tex->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
    tex->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
    tex->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP_TO_EDGE);
    tex->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::CLAMP_TO_EDGE);
    tex->setInternalFormat(internalFormat);
    tex->setUnRefImageDataAfterApply(false);

    return tex;
}

void ClearImage(osg::Image * image) {
    auto sizeInBytes = image->getTotalSizeInBytes();
    std::memset(image->data(), 0, sizeInBytes);
}

std::pair<float, float> GetTextureCoordsofParticle(osgParticle::Particle * particle) {
    auto &pos = particle->getPosition();
    auto spherePos = ConvertCartesianToSpherical(pos); //(r, inclination, azimuth)
    // in tex coordinates, inclination = s, azimuth = t
    float &inclination = spherePos.y();
    float &azimuth = spherePos.z();
    float s = MapToRange(inclination, 0.f, osg::PIf, 0.f, 1.f);
    float t = MapToRange(azimuth, -osg::PIf, osg::PIf, 0.f, 1.f);

    return std::make_pair(s, t);
}
