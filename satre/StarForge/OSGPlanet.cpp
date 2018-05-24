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
#include <osgParticle/Counter>
#include <osgParticle/ConstantRateCounter>
#include <osgParticle/SectorPlacer>
#include <osgParticle/ModularProgram>
#include <osgDB/FileUtils>
#include <osg/Program>
#include <osg/BlendFunc>
#include <osg/Depth>
#include <osg/Matrix>

#include <cvrConfig/ConfigManager.h>
#include <cvrKernel/PluginHelper.h>
#include <cvrKernel/CVRViewer.h>

int getestimatedMaxNumberOfParticles(osgParticle::ConstantRateCounter * counter, double lifetime);

OSGPlanet::OSGPlanet(size_t numRepulsors, size_t numAttractors, std::string & assetsDir) : mAssetsDir(assetsDir),
																						   mRoot(new osg::Group),
																						   mScaleNode(new osg::MatrixTransform),
																						   mPlanetDrawProgram(new osg::Program)
{
    InitParticleSystem(numRepulsors, numAttractors, assetsDir);

    // Load the shaders
//    auto shadersPath = cvr::ConfigManager::getEntry("value", "Plugin.StarForge.ShadersPath", "/home/satre/CVRPlugins/satre/StarForge/shaders/");
//    mVertexShader = osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile(shadersPath + "starforge.vert"));
//    mFragShader = osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile(shadersPath + "starforge.frag"));
//
//    if(mVertexShader != nullptr) {
//        std::cout << "Loaded vertex shader." << std::endl;
//    }
//    if(mFragShader != nullptr) {
//        std::cout << "Loaded fragment shader." << std::endl;
//    }
//
//    // Setup the drawing
//    auto stateset = mSystem->getOrCreateStateSet();
//    auto drawProgram = new osg::Program;
//    drawProgram->addShader(mVertexShader);
//    drawProgram->addShader(mFragShader);
//    stateset->setAttribute(drawProgram);
////    stateset->addUniform(new osg::Uniform(osg::Uniform::Type::FLOAT_MAT4, "osg_ModelViewProjectionMatrix"));
////    stateset->addUniform(new osg::Uniform(osg::Uniform::Type::FLOAT_MAT4, "osg_ModelViewMatrix"));
//    stateset->addUniform(new osg::Uniform(osg::Uniform::Type::FLOAT_MAT4, "osg_ViewMatrixInverse"));
////    stateset->addUniform(new osg::Uniform(osg::Uniform::Type::FLOAT_MAT4, "osg_NormalMatrix"));
//    stateset->addUniform(new osg::Uniform(osg::Uniform::Type::FLOAT_MAT4, "osg_ViewMatrix"));
//    stateset->addUniform(new osg::Uniform(osg::Uniform::Type ::FLOAT_MAT4, "osg_ModelMatrix"));
//    stateset->addUniform(new osg::Uniform(osg::Uniform::Type::FLOAT_MAT4, "osg_ProjectionMatrix"));

//    auto blend = new osg::BlendFunc();
//    blend->setFunction(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::ONE_MINUS_SRC_ALPHA);
//    stateset->setAttributeAndModes(blend, osg::StateAttribute::ON);
//
//    auto depth = new osg::Depth;
//    depth->setWriteMask(true);
//    depth->setFunction(osg::Depth::Function::LESS);
//    stateset->setAttributeAndModes(depth, osg::StateAttribute::ON);
}

OSGPlanet::~OSGPlanet() {
}

void OSGPlanet::InitParticleSystem(size_t numRepulsors, size_t numAttractors, std::string & assetsDir) {
    mRoot->addChild(mScaleNode);

    /// Init the particle system
    mSystem = new osgParticle::ParticleSystem;
//	mSystem->setDefaultAttributesUsingShaders();
    std::string assetsPath = cvr::ConfigManager::getEntry("value", "Plugin.StarForge.AssetsPath", "/home/satre/Assets/StarForge/");
    mSystem->setDefaultAttributes(assetsPath + "particle.png", true, false);
//	mSystem->setDefaultAttributesUsingShaders(assetsPath + "particle.png", true, false);

    // Init a template particle, which all future particles will be copies of.
    osgParticle::Particle pTemplate;
    pTemplate.setLifeTime(mParticleLifeTime); // 200 seconds of life.
//    pTemplate.setLifeTime(-1.f); // Particles never die.
    pTemplate.setMass(0.001f); // 1 gram of mass
    pTemplate.setRadius(500.f);
    mSystem->setDefaultParticleTemplate(pTemplate);

    // Init the emitter.
    auto * emitter = new osgParticle::ModularEmitter;
    emitter->setParticleSystem(mSystem);

    // Init the counter for the emitter
//    auto * counter = new osgParticle::RandomRateCounter;
//    counter->setRateRange(80, 120);
    auto * counter = new osgParticle::ConstantRateCounter;
    counter->setMinimumNumberOfParticlesToCreate(10);
    counter->setNumberOfParticlesPerSecondToCreate(10);
    std::cout << "Estimated max number of particles: "
                 << getestimatedMaxNumberOfParticles(counter, mParticleLifeTime) << std::endl;
    emitter->setCounter(counter);

    // Init the placer for the emitter
    osg::Vec3 center = osg::Vec3(params::gPlanetCenter.x, params::gPlanetCenter.y, params::gPlanetCenter.z);

    auto * placer = new SpherePlacer(center, params::gPlanetRadius);
    emitter->setPlacer(placer);

    // Init the shooter for the emitter
    auto * shooter = new SphereTangentShooter;
    emitter->setShooter(shooter);

    // Add the shooter to the scene graph.
    mScaleNode->addChild(emitter);

    // Create a program to control the post-spawning behavior of the particles
    osgParticle::ModularProgram * program = new osgParticle::ModularProgram;
    program->setParticleSystem(mSystem);

    program->addOperator(new PositionCorrectionOperator);


//	for (size_t i = 0; i < numAttractors; ++i) {
//		auto pos = RandomPointOnSphere();
//		AttractorVorton * v = new AttractorVorton(pos);
//		v->SetVorticity(RandomFloat(30.f));
//		program->addOperator(v);
//	}

    auto vortonsVertices = new osg::Vec3Array(numRepulsors);
    auto vortonsColors = new osg::Vec3Array(numRepulsors);
    for (size_t i = 0; i < numRepulsors; ++i) {
        auto pos = RandomPointOnSphere() * params::gPlanetRadius;
        RepulsorVorton * v = new RepulsorVorton(pos);
        v->SetVorticity(RandomFloat(30.f));
        program->addOperator(v);

        (*vortonsVertices)[i].set(v->GetPosition().x, v->GetPosition().y, v->GetPosition().z);
        auto color = glm::normalize(v->GetPosition());
        (*vortonsColors)[i].set(color.r, color.g, color.b);
    }


    auto vortonsGeom = new osg::Geometry;
    vortonsGeom->setVertexArray(vortonsVertices);
    vortonsGeom->setColorArray(vortonsColors);
    vortonsGeom->setColorBinding(osg::Geometry::BIND_OVERALL);
    vortonsGeom->addPrimitiveSet(new osg::DrawArrays(GL_POINTS, 0, numRepulsors));
    vortonsGeom->getOrCreateStateSet()->setAttribute(new osg::Point(12.f), osg::StateAttribute::ON);
    auto vortonsGeode = new osg::Geode;
    vortonsGeode->addDrawable(vortonsGeom);


    // Add the program to the scene graph
    mScaleNode->addChild(program);

    // Create a drawable target for the particle system
    auto geode = new osg::Geode;
    geode->setCullingActive(false);
    geode->addDrawable(mSystem);
    mScaleNode->addChild(geode);

    mScaleNode->addChild(vortonsGeode);

    // Create a particle system updater
    auto psUpdater = new osgParticle::ParticleSystemUpdater;
    psUpdater->addParticleSystem(mSystem);

    mScaleNode->addChild(psUpdater);
}

void OSGPlanet::PreFrame() {
//	auto & objectMatrix = cvr::PluginHelper::getObjectMatrix();
//	auto & viewMat = cvr::CVRViewer::instance()->getCamera()->getViewMatrix();
//	auto & projMat = cvr::CVRViewer::instance()->getCamera()->getProjectionMatrix();
//	auto inverseViewMat = cvr::CVRViewer::instance()->getCamera()->getInverseViewMatrix();
//
//	mSystem->getOrCreateStateSet()->getUniform("osg_ModelMatrix")->set(objectMatrix);
//	mSystem->getOrCreateStateSet()->getUniform("osg_ViewMatrix")->set(viewMat);
//	mSystem->getOrCreateStateSet()->getUniform("osg_ViewMatrixInverse")->set(inverseViewMat);
//	mSystem->getOrCreateStateSet()->getUniform("osg_ProjectionMatrix")->set(projMat);
//    std::cout << mSystem->numParticles() << std::endl;

}

void OSGPlanet::PostFrame() {

}

int getestimatedMaxNumberOfParticles(osgParticle::ConstantRateCounter * counter, double lifetime) {
    int minNumParticles =  static_cast<int>(counter->getMinimumNumberOfParticlesToCreate() * 60.0f * lifetime);
    int baseNumPartciles = static_cast<int>(counter->getNumberOfParticlesPerSecondToCreate() * lifetime);
    return osg::maximum(minNumParticles, baseNumPartciles);
}