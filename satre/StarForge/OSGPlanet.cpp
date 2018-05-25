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
#include <unordered_map>

#include <cvrConfig/ConfigManager.h>
#include <cvrKernel/PluginHelper.h>
#include <cvrKernel/CVRViewer.h>

//int getestimatedMaxNumberOfParticles(osgParticle::ConstantRateCounter * counter, double lifetime);
osg::Image * CreateImage(int width, int height, int numComponents);
osg::Texture2D * CreateTexture(int width, int height, int numComponents);
void ClearImage(osg::Image * image);

OSGPlanet::OSGPlanet(size_t numRepulsors, size_t numAttractors, std::string & assetsDir) : mAssetsDir(assetsDir),
																						   mRoot(new osg::MatrixTransform)
{
    InitParticleSystem(numRepulsors, numAttractors, assetsDir);
    InitPlanetGeometry();
}

OSGPlanet::~OSGPlanet() = default;

void OSGPlanet::InitParticleSystem(size_t numRepulsors, size_t numAttractors, std::string & assetsDir) {

    auto shadersPath = cvr::ConfigManager::getEntry("value", "Plugin.StarForge.ShadersPath", "/home/satre/CVRPlugins/satre/StarForge/shaders/");

    /// Init the particle system
    mSystem = new osgParticle::ParticleSystem;
    std::string assetsPath = cvr::ConfigManager::getEntry("value", "Plugin.StarForge.AssetsPath", "/home/satre/Assets/StarForge/");
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
    counter->setMinimumNumberOfParticlesToCreate(3);
    counter->setNumberOfParticlesPerSecondToCreate(5);
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
    mRoot->addChild(mParticleEmitter);

    // Create a program to control the post-spawning behavior of the particles
    auto * program = new osgParticle::ModularProgram;
    program->setParticleSystem(mSystem);
    program->addOperator(new PositionCorrectionOperator);


    osg::ref_ptr<osg::Vec3Array> vortonsVertices = new osg::Vec3Array;
    osg::ref_ptr<osg::Vec4Array> vortonsColors = new osg::Vec4Array;
	for (size_t i = 0; i < (numAttractors + numRepulsors); i++) {
        auto pos = RandomPointOnSphere() * params::gPlanetRadius;
        Vorton * v;
        if(i < numAttractors) {
            v = new AttractorVorton(pos);
        } else {
            v = new RepulsorVorton(pos);
        }
        v->SetVorticity(RandomFloat(5.f));
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
    mPlanetSphere = new osg::ShapeDrawable(
            new osg::Sphere(GLM2OSG(params::gPlanetCenter), params::gPlanetRadius));

    auto colorArray = new osg::Vec4Array;
    colorArray->push_back(osg::Vec4(0.3f, 0.3f, 0.76f, 0.3f));
    mPlanetSphere->setColorArray(colorArray, osg::Array::BIND_OVERALL);
    mPlanetSphere->setUseVertexBufferObjects(true);
    mPlanetSphere->setUseVertexArrayObject(true);

    // Create the Geode (Geometry Node)
    auto geode = new osg::Geode;
    geode->addDrawable(mPlanetSphere);

    // Setup the textures that will hold the particle data
    auto stateset = geode->getOrCreateStateSet();
    auto * counter = dynamic_cast<osgParticle::ConstantRateCounter*>( mParticleEmitter->getCounter());

    int texSize = int(std::ceil(fsqrtf(counter->getEstimatedMaxNumOfParticles(mParticleLifeTime)))) + 1; // Err on the side of caution
    std::cerr << "Initializing textures with width " << texSize << std::endl;

    {
        mColorTexture = CreateTexture(texSize, texSize, 4);
        auto image = CreateImage(texSize, texSize, 4);
        mColorTexture->setImage(image);
        stateset->setTextureAttributeAndModes(0, mColorTexture, osg::StateAttribute::ON);
    }

    {
        mAgeVelocityTexture = CreateTexture(texSize, texSize, 4);
        auto image = CreateImage(texSize, texSize, 4);
        mAgeVelocityTexture->setImage(image);
        stateset->setTextureAttributeAndModes(1, mAgeVelocityTexture, osg::StateAttribute::ON);
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
    UpdateColorDataTexture();
}

void OSGPlanet::PostFrame() {

}

void OSGPlanet::UpdateColorDataTexture() {
    auto image = mColorTexture->getImage();
    auto texWidth = image->s();

    if (!image->isDataContiguous()) std::cerr << "Image data is not contiguous!" << std::endl;
    auto data = reinterpret_cast<float *>(image->data());

    std::unordered_map<std::pair<int, int>, int> contribCount;

    // Every frame, refill the textures with the particle data.
    // assume column major: x * height + y
    int numParticles = mSystem->numParticles();
    for (int i = 0; i < numParticles; ++i) {
        // Get a particle and convert its pos into spherical coords.
        auto particle = mSystem->getParticle(i);
        auto &pos = particle->getPosition();
        auto spherePos = ConvertCartesianToSpherical(pos); //(r, inclination, azimuth)

        // in tex coordinates, inclination = s, azimuth = t
        float &inclination = spherePos.y();
        float &azimuth = spherePos.z();
        float s = MapToRange(inclination, 0.f, osg::PIf, 0.f, 1.f);
        float t = MapToRange(azimuth, -osg::PIf, osg::PIf, 0.f, 1.f);
//        assert(s >= 0.f && s <= 1.f);
//        assert(t >= 0.f && t <= 1.f);

        // Convert from texture coordinates to image coordinates
        auto x = int(std::floor(s * image->s()));
        auto y = int(std::floor(t * image->t()));

        if(contribCount.find(std::make_pair(x, y)) == contribCount.end()) {
            contribCount.insert(std::make_pair(std::make_pair(x, y), 1));
        } else {
            (contribCount.at(std::make_pair(x, y)))++;
        }

        int index = (x * image->t() + y) * 4;
//        assert(index < image->getTotalDataSize());
//        assert(index + 1 < image->getTotalDataSize());
//        assert(index + 2 < image->getTotalDataSize());
//        assert(index + 3 < image->getTotalDataSize());
        data[index] += particle->getCurrentColor().r();
        data[index+1] += particle->getCurrentColor().g();
        data[index+2] += particle->getCurrentColor().b();
        data[index+3] += particle->getCurrentColor().a();
    }

    // Average out color data
    for (int y = 0; y < image->t(); ++y) {
        for (int x = 0; x < image->s(); ++x) {
            if(contribCount.find(std::make_pair(x, y)) != contribCount.end()) {
                auto numContribs = float(contribCount.at(std::make_pair(x, y)));

                int i = (x * image->t() + y) * 4;
                data[i] /= numContribs; data[i+1] /= numContribs;
                data[i+2] /= numContribs; data[i+3] /= numContribs;
            }
        }
    }

    // mark for upload
    image->dirty();
}

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

}