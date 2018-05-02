#include "AttractorVorton.hpp"
#include "math_helper.hpp"
#include "GlobalParameters.hpp"

#include <glm/gtx/norm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <osg/Vec3>

// Default to exponential dropoff.
const std::function<float(const float &, float)> AttractorVorton::sDefaultDropoffFn = [](const float & maxV, float dist) {
	return std::exp(-dist / 6.f) * maxV;
};

// TODO: Default exponential ejection
const std::function<glm::vec3(const AttractorVorton &, const glm::vec3 &)> AttractorVorton::sDefaultEjectionFn = [](const AttractorVorton & vorton, const glm::vec3 & samplePoint) {
	// The dist b/w sample point is assumed to be < vorton radius.

	// Figure out the distance from the center of the vorton and normalize to [0,1]
	// Use square distance, as it's faster, and factor into exponential
	const float radius2 = vorton.mRadius * vorton.mRadius;
	float distNorm2 = MapToRange(glm::length2(samplePoint - vorton.mPosition), 0.f, radius2, 0.f, 1.f);

	// Ejection vector points away from vorton.
	glm::vec3 ejectionVec = glm::normalize(samplePoint - vorton.mPosition);
	ejectionVec = ejectionVec * std::exp(-5.f * distNorm2 / 2.f) * vorton.mVorticity;
	return ejectionVec;
};

AttractorVorton::AttractorVorton() : Vorton(), mDropoffFn(sDefaultDropoffFn), mEjectionFn(sDefaultEjectionFn), mRadius(0.01f), mAngularSpeed(0.005f)
{
	// Create a random vector and take it's cross product to get a vec that is
	// orthogonal to the position vector.
	auto randVec = RandomPointOnSphere();
	mRotationAxis = glm::normalize(glm::cross(randVec, mPosition));
}

AttractorVorton::AttractorVorton(glm::vec3 & pos, float azDelta, float elevDelta) :
	Vorton(pos, azDelta, elevDelta), mDropoffFn(sDefaultDropoffFn), mEjectionFn(sDefaultEjectionFn),
	mRadius(0.01f), mAngularSpeed(0.005f)
{
	// Create a random vector and take it's cross product to get a vec that is
	// orthogonal to the position vector.
	auto randVec = RandomPointOnSphere();
	mRotationAxis = glm::normalize(glm::cross(randVec, mPosition));
}

AttractorVorton::AttractorVorton(const AttractorVorton & other,  const osg::CopyOp & copyOp) :
Vorton(other, copyOp), mDropoffFn(other.mDropoffFn), mEjectionFn(other.mEjectionFn),
mRadius(other.mRadius), mAngularSpeed(other.mAngularSpeed), mRotationAxis(other.mRotationAxis)
{}

glm::vec3 AttractorVorton::ComputeForceVector(const glm::vec3 & samplePoint) const {
	// The domain of force vectors is the 2D plane tangent to the sample point on the sphere.

	//------
	// 1. Convert the sample point and vorton pos to world space Cartesian coords.
	const auto & sampleCartesian = samplePoint;
	const auto & posCartesian = mPosition;

	//------
	// 2. Calculate the force vector.
	// The force vector is orthogonal to the relative position of the sample point.
	// It is also tangential to the circle centered at the vorton pos whose edge passes through
	// the sample point.
	// TODO: This isn't strictly speaking valid, the correct way would be to use the length of the arc
	// b/w the sample point and the vorton position on the surface of the sphere.
	float dist = glm::distance(sampleCartesian, posCartesian);
	glm::vec3 forceVec(1.f);
	if (dist <= mRadius) { // Tracer is inside vorton.
		return mEjectionFn(*this, sampleCartesian);
	}
	else {
		// Force vec tangential to vorton plane and orthogonal to sphere, so generate using cross product
		forceVec = glm::normalize(glm::cross(sampleCartesian - posCartesian, posCartesian)); //Switch order for CW or CCW spin
		//Apply magnitude
		forceVec = mDropoffFn(mVorticity, dist) * forceVec;
		// If force vec is really small, just return zero.
		if (glm::length2(forceVec) <= 0.00001f) return glm::vec3(0.f);
	}

	return forceVec;
}

void AttractorVorton::Update(const float timeStep) {
	mPosition = glm::rotate(mPosition, mAngularSpeed, mRotationAxis);
}

void AttractorVorton::beginOperate(osgParticle::Program * prog) {

}
void AttractorVorton::operate(osgParticle::Particle * particle, double dt) {
	osg::Vec3 forceVec = GLM2OSG(ComputeForceVector(OSG2GLM(particle->getPosition())));
	auto accel = forceVec / particle->getMass();
	particle->addVelocity(accel * dt);
}