#pragma once

#include <osgParticle/Shooter>
#include <osg/Group>
#include <osg/Object>

class SphereTangentShooter: public osgParticle::Shooter
{
public:
	SphereTangentShooter();
	SphereTangentShooter(const SphereTangentShooter & other, const osg::CopyOp & copyOp = osg::CopyOp::SHALLOW_COPY);

	META_Object(osgParticle, SphereTangentShooter);

	void shoot(osgParticle::Particle * p) const override;

	~SphereTangentShooter() {}
	
};