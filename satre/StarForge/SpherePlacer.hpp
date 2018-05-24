#pragma once

#include <osgParticle/Placer>
#include <osgParticle/CenteredPlacer>
#include "math_helper.hpp"

/// Randomly places a particle on the surface of a sphere specified by the radius.
class SpherePlacer: public osgParticle::CenteredPlacer {
public:


    SpherePlacer(osg::Vec3 pos = osg::Vec3(0, 0, 0), float rad = 100.f);
    SpherePlacer(const SpherePlacer& copy, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);
    META_Object(osgParticle, SpherePlacer);
    virtual void place( osgParticle::Particle * P) const override;
    virtual float volume() const override;
    virtual osg::Vec3 getControlPosition() const override ;
    float GetRadius() const { return mRadius; }
    void SetRadius(float _radius) { mRadius = _radius; }
private:
    float mRadius;
};


SpherePlacer::SpherePlacer(osg::Vec3 pos, float rad) :
        CenteredPlacer(), mRadius(rad) {
    setCenter(pos);
}

SpherePlacer::SpherePlacer(const SpherePlacer &copy, const osg::CopyOp &copyop) :
        CenteredPlacer(copy, copyop), mRadius(copy.mRadius)
{}

void SpherePlacer::place(osgParticle::Particle *P) const {
    P->setPosition(GLM2OSG(RandomPointOnSphere()) * mRadius);
}

float SpherePlacer::volume() const {
    return 4.f / 3.f * osg::PI * mRadius * mRadius * mRadius;
}

osg::Vec3 SpherePlacer::getControlPosition() const {
    return getCenter();
}