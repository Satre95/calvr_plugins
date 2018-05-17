#include "OSGPlanet.hpp"
#include "GlobalParameters.hpp"
#include "SphereTangentShooter.hpp"
#include "AttractorVorton.hpp"
#include "RepulsorVorton.hpp"
#include "math_helper.hpp"
#include "SpherePlacer.hpp"

#include <osg/Geometry>
#include <osg/Geode>
#include <osg/Point>
#include <osgParticle/ModularEmitter>
#include <osgParticle/RandomRateCounter>
#include <osgParticle/SectorPlacer>
#include <osgParticle/ModularProgram>
#include <cvrConfig/ConfigManager.h>

OSGPlanet::OSGPlanet(size_t numRepulsors, size_t numAttractors, std::string & assetsDir) : mAssetsDir(assetsDir),
																						   mRoot(new osg::Group),
																						   mScaleNode(new osg::MatrixTransform),
																						   mPlanetDrawProgram(new osg::Program)
{
	mRoot->addChild(mScaleNode);

	/// Init the particle system
	mSystem = new osgParticle::ParticleSystem;
//	mSystem->setDefaultAttributesUsingShaders();
	std::string assetsPath = cvr::ConfigManager::getEntry("value", "Plugin.StarForge.AssetsPath", "/home/satre/Assets/StarForge/");
	mSystem->setDefaultAttributes(assetsPath + "particle.png", true, false);

	// Init a template particle, which all future particles will be copies of.
	osgParticle::Particle pTemplate;
	pTemplate.setLifeTime(200); // 500 seconds of life.
	pTemplate.setMass(0.001f); // 1 gram of mass
	pTemplate.setRadius(500.f);
	mSystem->setDefaultParticleTemplate(pTemplate);
	//mSystem->getDefaultParticleTemplate().setShape(osgParticle::Particle::LINE);

	// Init the emitter.
	auto * emitter = new osgParticle::ModularEmitter;
	emitter->setParticleSystem(mSystem);

	// Init the counter for the emitter
	auto * counter = new osgParticle::RandomRateCounter;
	counter->setRateRange(80, 120);
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

OSGPlanet::~OSGPlanet() {
}