#pragma once

#include <osgParticle/Operator>
#include "GlobalParameters.hpp"
#include "math_helper.hpp"

class PositionCorrectionOperator: public osgParticle::Operator {
public:
    PositionCorrectionOperator() : osgParticle::Operator() {}

    PositionCorrectionOperator(const PositionCorrectionOperator & other, const osg::CopyOp & copyOp = osg::CopyOp::SHALLOW_COPY) :
            osgParticle::Operator(other, copyOp) {
    }

    void beginOperate(osgParticle::Program * prog) override {}
    void operate(osgParticle::Particle * particle, double dt) override {
        // Get the center of the planet in the world
        auto center = GLM2OSG(params::gPlanetCenter);
        const auto & radius = params::gPlanetRadius;

        // Project the position onto surface of unit sphere at origin and then translate
        auto pos = particle->getPosition();
        pos.normalize();
        pos = pos * radius;
        pos = pos + center;
        particle->setPosition(pos);
    }

    META_Object(osgParticle, PositionCorrectionOperator);

    virtual ~PositionCorrectionOperator() {}
};