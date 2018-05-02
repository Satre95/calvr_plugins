#include "SphereTangentShooter.hpp"
#include <osgParticle/Particle>
#include "math_helper.hpp"
SphereTangentShooter::SphereTangentShooter() {
}

SphereTangentShooter::SphereTangentShooter(const SphereTangentShooter & other, const osg::CopyOp & copyOp) :
osgParticle::Shooter(other, copyOp)
{

}

void SphereTangentShooter::shoot(osgParticle::Particle * p) const {
	// Get the position of the particle
	auto & pos = p->getPosition();

	//Assuming the particle is on the surface of the sphere, get a random vector that is in the tangential plane.

	//Generate a random vector and calc its cross product with the pos to get a particle on the tangent plane.
	osg::Vec3 randVec = GLM2OSG(RandomPointOnSphere());
	osg::Vec3 vel = randVec ^ pos;
	vel.normalize();

	p->setVelocity(vel);
}