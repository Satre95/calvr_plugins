#include "OSGPlanet.hpp"
#include "GlobalParameters.hpp"
#include "SphereTangentShooter.hpp"
#include "AttractorVorton.hpp"
#include "RepulsorVorton.hpp"
#include "math_helper.hpp"

#include <osgParticle/ModularEmitter>
#include <osgParticle/RandomRateCounter>
#include <osgParticle/SectorPlacer>
#include <osgParticle/ModularProgram>
#include <osgParticle/FluidFrictionOperator>

OSGPlanet::OSGPlanet(size_t numRepulsors, size_t numAttractors, std::string & assetsDir) : mAssetsDir(assetsDir), mRoot(new osg::Group)
{

	/// Init the particle system
	mSystem = new osgParticle::ParticleSystem;
	mSystem->setDefaultAttributesUsingShaders();

	// Init a template particle, which all future particles will be copies of.
	osgParticle::Particle pTemplate;
	pTemplate.setLifeTime(5); // 5 seconds of life.
	pTemplate.setMass(0.001f); // 1 gram of mass
	mSystem->setDefaultParticleTemplate(pTemplate);

	// Init the emitter.
	osgParticle::ModularEmitter * emitter = new osgParticle::ModularEmitter;
	emitter->setParticleSystem(mSystem);

	// Init the counter for the emitter
	osgParticle::RandomRateCounter * counter = new osgParticle::RandomRateCounter;
	counter->setRateRange(60, 60);
	emitter->setCounter(counter);

	// Init the placer for the emitter
	osgParticle::SectorPlacer * placer = new osgParticle::SectorPlacer;
	placer->setCenter(params::gPlanetCenter.x, params::gPlanetCenter.y, params::gPlanetCenter.z);
	placer->setRadiusRange(params::gPlanetRadius, params::gPlanetRadius);
	placer->setPhiRange(0, 2.f * osg::PI);
	emitter->setPlacer(placer);

	// Init the shooter for the emitter
	SphereTangentShooter * shooter = new SphereTangentShooter;
	emitter->setShooter(shooter);

	// Add the shooter to the scene graph.
	mRoot->addChild(emitter);

	// Create a program to control the post-spawning behavior of the particles
	osgParticle::ModularProgram * program = new osgParticle::ModularProgram;
	program->setParticleSystem(mSystem);

	for (size_t i = 0; i < numAttractors; ++i) {
		auto pos = RandomPointOnSphere();
		AttractorVorton * v = new AttractorVorton(pos);
		v->SetVorticity(RandomFloat(30.f));
		program->addOperator(v);
	}

	for (size_t i = 0; i < numRepulsors; ++i) {
		auto pos = RandomPointOnSphere();
		RepulsorVorton * v = new RepulsorVorton(pos);	
		v->SetVorticity(RandomFloat(30.f));
		program->addOperator(v);
	}

	osgParticle::FluidFrictionOperator * ffop = new osgParticle::FluidFrictionOperator;
	ffop->setFluidToAir();
	program->addOperator(ffop);

	// Add the program to the scene graph
	mRoot->addChild(program);

	// Add the particle system to the scene graph
	mRoot->addChild(mSystem);
}

OSGPlanet::~OSGPlanet() {

}