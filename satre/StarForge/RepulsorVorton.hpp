#pragma once

#include "Vorton.hpp"
#include <glm/glm.hpp>
#include <functional>

class RepulsorVorton : public Vorton {
public:
	RepulsorVorton();
	RepulsorVorton(glm::vec3 & pos, float azDelta = glm::radians(90.f), float elevDelta = glm::radians(90.f));
	RepulsorVorton(const RepulsorVorton & other, const osg::CopyOp & copyOp = osg::CopyOp::SHALLOW_COPY);
	META_Object(osgParticle, RepulsorVorton);

	glm::vec3 ComputeForceVector(const glm::vec3 & pos) const override;
    void Update(const float timeStep) override;
    
	void SetDropoffFn(std::function<float(const float &, float)> & fn) { mDropoffFn = fn; }
    
	void operate(osgParticle::Particle * particle, double dt) override;
private:

	/// The function used to control the vorticity dropoff.
	/// 1st param is max vorticity.
	/// 2nd param is the distance from the vorton.
	/// Returns the scaled vorticity.
	std::function<float(const float &, float)> mDropoffFn;

	static const std::function<float(const float &, float)> sDefaultDropoffFn;
};
