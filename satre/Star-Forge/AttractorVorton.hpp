#pragma once

#include "Vorton.hpp"
#include <functional>

/**
 \brief An Attractor vorton creates a swirling vortex around it.

 The force vectors induced by such vortons are orthogonal to both the relative position vector
 between the agent and vorton and the surface of the sphere.
*/
class AttractorVorton : public Vorton {
public:
	AttractorVorton();
	AttractorVorton(glm::vec3 & pos, float azDelta = glm::radians(90.f), float elevDelta = glm::radians(90.f));
	AttractorVorton(const AttractorVorton & other, const osg::CopyOp & copyOp = osg::CopyOp::SHALLOW_COPY);

	META_Object(osgParticle, AttractorVorton);

	glm::vec3 ComputeForceVector(const glm::vec3 & pos) const override;
	void Update(const float timeStep) override;
	void SetRotationAxis(glm::vec3 axis) { mRotationAxis = axis; }
	const glm::vec3 & GetRotationAxis() const { return mRotationAxis; }
	glm::vec3 & GetRotationAxis() { return mRotationAxis; }

	void beginOperate(osgParticle::Program * prog) override;
	void operate(osgParticle::Particle * particle, double dt) override;
private:
	/// The function used to control the vorticity dropoff.
	/// 1st param is max vorticity.
	/// 2nd param is the distance from the vorton.
	/// Returns the scaled vorticity.
	std::function<float(const float &, float)> mDropoffFn;

	/// A function that returns the ejection force vector for when
	/// a tracer enters the vorton.
	std::function<glm::vec3(const AttractorVorton &, const glm::vec3 &)> mEjectionFn;

	static const std::function<glm::vec3(const AttractorVorton &, const glm::vec3 &)> sDefaultEjectionFn;
	static const std::function<float(const float &, float)> sDefaultDropoffFn;

	/// To prevent singularities, attractor vortons have a non-zero radius.
	/// The vorton will eject tracers that enter this radius.
	float mRadius;

	/// The (scalar) speed at which this vorton moves across the sphere.
	float mAngularSpeed;
	/// The rotation axis. In order to preserve sphere, it MUST be orthogonal to the
	/// initial position vector relative to center of the sphere.
	/// MUST also be normalized.
	glm::vec3 mRotationAxis;
};