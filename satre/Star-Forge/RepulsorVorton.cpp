#include "RepulsorVorton.hpp"
#include "math_helper.hpp"
#include "GlobalParameters.hpp"

#include <glm/gtx/norm.hpp>

const std::function<float(const float &, float)> RepulsorVorton::sDefaultDropoffFn = [](const float & maxV, float dist) {
	return std::exp(-dist / 4.f) * maxV;
};

RepulsorVorton::RepulsorVorton() : Vorton(), mDropoffFn(sDefaultDropoffFn)
{}

RepulsorVorton::RepulsorVorton(glm::vec3 & pos, float azDelta, float elevDelta) :
	Vorton(pos, azDelta, elevDelta), mDropoffFn(sDefaultDropoffFn)
{}

RepulsorVorton::RepulsorVorton(const RepulsorVorton & other, const osg::CopyOp & copyOp) :
	Vorton(other, copyOp), mDropoffFn(other.mDropoffFn)
{}

glm::vec3 RepulsorVorton::ComputeForceVector(const glm::vec3 & samplePoint) const {
	// The domain of force vectors is the 2D plane tangent to the sample point on the sphere.

	//------
	// 1. Convert the sample point and vorton pos to world space Cartesian coords.
	auto sampleCartesian = samplePoint;
	auto posCartesian = mPosition;

	//------
	// 2. Determine the plane between the sample point and the vorton pos.
	// The plane normal is the vector from center of planet to midpoint of line segment
	// b/w samplePos and vortonPos
	//auto midpoint = Midpoint(sampleCartesian, posCartesian);
	//auto planeNorm = glm::normalize(midpoint - params::gPlanetCenter);

	//------
	// 3. Calculate the force vector.
	// The force vector points away from the vorton towards the sample point.
	// TODO: This isn't strictly speaking valid, the correct way would be to use the length of the arc
	// b/w the sample point and the vorton position on the surface of the sphere.
	float dist = glm::distance(sampleCartesian, posCartesian);
	auto forceVec = glm::normalize(sampleCartesian - posCartesian) * mDropoffFn(mVorticity, dist);
	// If force vec is really small, just return zero.
	if (glm::length2(forceVec) <= 0.00001f) return glm::vec3(0.f);

	//------
	// 3. Project the force vec onto the tangent plane at the sample point, preserving
	// the magnitude.
	// The normal of the tangent plane is the local space sample position vector
	float forceMag = glm::length(forceVec);
	glm::vec3 samplePlaneNorm = glm::normalize(sampleCartesian);
	glm::vec3 projectedForce = ProjectVectorOnPlane(forceVec, samplePlaneNorm);
	projectedForce = glm::normalize(projectedForce) * forceMag;

	return projectedForce;
}

void RepulsorVorton::Update(const float timeStep) {
    
}

void RepulsorVorton::beginOperate(osgParticle::Program * prog) {

}
void RepulsorVorton::operate(osgParticle::Particle * particle, double dt) {
	osg::Vec3 forceVec = GLM2OSG(ComputeForceVector(OSG2GLM(particle->getPosition())));
	auto accel = forceVec / particle->getMass();
	particle->addVelocity(accel * dt);
}