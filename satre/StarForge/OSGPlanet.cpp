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
#include <osg/Shape>
#include <osg/ShapeDrawable>
#include <osg/Texture2D>
#include <osg/BlendFunc>
#include <osg/Depth>
#include <osg/GLExtensions>

#include <cvrConfig/ConfigManager.h>
#include <cvrKernel/PluginHelper.h>
#include <cvrKernel/CVRViewer.h>

int getestimatedMaxNumberOfParticles(osgParticle::ConstantRateCounter * counter, double lifetime);

OSGPlanet::OSGPlanet(size_t numRepulsors, size_t numAttractors, std::string & assetsDir) : mAssetsDir(assetsDir),
																						   mRoot(new osg::MatrixTransform),
																						   mPlanetDrawProgram(new osg::Program)
{
    InitParticleSystem(numRepulsors, numAttractors, assetsDir);
//    InitPlanetGeometry();
}

OSGPlanet::~OSGPlanet() {
}

void OSGPlanet::InitParticleSystem(size_t numRepulsors, size_t numAttractors, std::string & assetsDir) {

    auto shadersPath = cvr::ConfigManager::getEntry("value", "Plugin.StarForge.ShadersPath", "/home/satre/CVRPlugins/satre/StarForge/shaders/");

    /// Init the particle system
    mSystem = new osgParticle::ParticleSystem;
    std::string assetsPath = cvr::ConfigManager::getEntry("value", "Plugin.StarForge.AssetsPath", "/home/satre/Assets/StarForge/");
    mSystem->setDefaultAttributes(assetsPath + "particle.png", true, false);

    // Init a template particle, which all future particles will be copies of.
    osgParticle::Particle pTemplate;
    pTemplate.setLifeTime(mParticleLifeTime);
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
    counter->setMinimumNumberOfParticlesToCreate(3);
    counter->setNumberOfParticlesPerSecondToCreate(15);
    mEstimatedMaxParticles = getestimatedMaxNumberOfParticles(counter, mParticleLifeTime);
    std::cout << "Estimated max number of particles: " << mEstimatedMaxParticles << std::endl;
    emitter->setCounter(counter);

    // Init the placer for the emitter
    osg::Vec3 center = GLM2OSG(params::gPlanetCenter);

    auto * placer = new SpherePlacer(center, params::gPlanetRadius);
    emitter->setPlacer(placer);

    // Init the shooter for the emitter
    auto * shooter = new SphereTangentShooter;
    emitter->setShooter(shooter);

    // Add the shooter to the scene graph.
    mRoot->addChild(emitter);

    // Create a program to control the post-spawning behavior of the particles
    osgParticle::ModularProgram * program = new osgParticle::ModularProgram;
    program->setParticleSystem(mSystem);

    program->addOperator(new PositionCorrectionOperator);


    auto vortonsVertices = new osg::Vec3Array;
    auto vortonsColors = new osg::Vec4Array;
	for (size_t i = 0; i < (numAttractors + numRepulsors); i++) {
        auto pos = RandomPointOnSphere() * params::gPlanetRadius;
        Vorton * v;
        if(i < numAttractors) {
            v = new AttractorVorton(pos);
        } else {
            v = new RepulsorVorton(pos);
        }
        v->SetVorticity(RandomFloat(10.f));
        program->addOperator(v);

        vortonsVertices->push_back(GLM2OSG(v->GetPosition()));
        osg::Vec4 color;
        if (i < numAttractors) {
            color = osg::Vec4(0.f, 1.f, 0.f, 1.f);
        } else {
            color = osg::Vec4(1.f, 0.f, 0.f, 1.f);
        }
        vortonsColors->push_back(color);

    }
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
    auto vertexShader = osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile(shadersPath + "particleDebug.vert"));
    auto fragShader = osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile(shadersPath + "particleDebug.frag"));
    drawProgram->addShader(vertexShader);
    drawProgram->addShader(fragShader);
    vortonsGeode->getOrCreateStateSet()->setAttribute(drawProgram, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

    mRoot->addChild(vortonsGeode);

    // Add the program to the scene graph
    mRoot->addChild(program);

    // Create a drawable target for the particle system
    auto geode = new osg::Geode;
    geode->setCullingActive(false);
    geode->addDrawable(mSystem);
    // Recycle the vortons geode stateset for particle debug draw.
    geode->setStateSet(vortonsGeode->getOrCreateStateSet());


    mRoot->addChild(geode);

    // Create a particle system updater
    auto psUpdater = new osgParticle::ParticleSystemUpdater;
    psUpdater->addParticleSystem(mSystem);

    mRoot->addChild(psUpdater);
}

void OSGPlanet::InitPlanetGeometry() {
    // Create the sphere
    auto sphereDrawable = new osg::ShapeDrawable(
            new osg::Sphere(GLM2OSG(params::gPlanetCenter), params::gPlanetRadius));

    auto colorArray = new osg::Vec4Array;
    colorArray->push_back(osg::Vec4(0.3f, 0.3f, 0.76f, 0.3f));
    sphereDrawable->setColorArray(colorArray, osg::Array::BIND_OVERALL);
    sphereDrawable->setUseVertexBufferObjects(true);
    sphereDrawable->setUseVertexArrayObject(true);

    // Create the Geode (Geometry Node)
    auto geode = new osg::Geode;
    geode->addDrawable(sphereDrawable);

    // Setup the textures that will hold the particle data
    auto stateset = geode->getOrCreateStateSet();
    int texSize = static_cast<int>(std::ceil(std::sqrt(mEstimatedMaxParticles)));

    {
        auto posTexture = new osg::Texture2D;
        posTexture->setDataVariance(osg::Object::DYNAMIC);
        posTexture->setTextureSize(texSize, texSize); // Set to upper bound of square texture
        posTexture->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
        posTexture->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
        posTexture->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP_TO_EDGE);
        posTexture->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::CLAMP_TO_EDGE);
        posTexture->setInternalFormat(GL_RGBA32F_ARB);
        stateset->setTextureAttributeAndModes(0, posTexture, osg::StateAttribute::ON);
    }

    {
        auto velTexture = new osg::Texture2D;
        velTexture->setDataVariance(osg::Object::DYNAMIC);
        velTexture->setTextureSize(texSize, texSize); // Set to upper bound of square texture
        velTexture->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
        velTexture->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
        velTexture->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP_TO_EDGE);
        velTexture->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::CLAMP_TO_EDGE);
        velTexture->setInternalFormat(GL_RGBA32F_ARB);
        stateset->setTextureAttributeAndModes(1, velTexture, osg::StateAttribute::ON);
    }

    // Load the shaders
    auto shadersPath = cvr::ConfigManager::getEntry("value", "Plugin.StarForge.ShadersPath", "/home/satre/CVRPlugins/satre/StarForge/shaders/");
    auto vertexShader = osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile(shadersPath + "starforge.vert"));
    auto fragShader = osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile(shadersPath + "starforge.frag"));

    if(!vertexShader) {
        std::cerr << "ERROR: Unable to load vertex shader in " << shadersPath << std::endl;
        return;
    }
    if(!fragShader) {
        std::cerr << "ERROR: Unable to load fragment shader in " << shadersPath << std::endl;
        return;
    }

    // Setup the programmable pipeline
    auto drawProgram = new osg::Program;
    drawProgram->addShader(vertexShader);
    drawProgram->addShader(fragShader);
    stateset->setAttribute(drawProgram, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
    // Add it to the scene graph
    mRoot->addChild(geode);
}

void OSGPlanet::PreFrame() {
}

void OSGPlanet::PostFrame() {

}

int getestimatedMaxNumberOfParticles(osgParticle::ConstantRateCounter * counter, double lifetime) {
    int minNumParticles =  static_cast<int>(counter->getMinimumNumberOfParticlesToCreate() * 60.0f * lifetime);
    int baseNumPartciles = static_cast<int>(counter->getNumberOfParticlesPerSecondToCreate() * lifetime);
    return osg::maximum(minNumParticles, baseNumPartciles);
}