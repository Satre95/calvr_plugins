#pragma once
#include <glm/glm.hpp>
#include <memory>
#include <osgParticle/Operator>
#include <osgParticle/ModularProgram>
#include <osgParticle/Particle>
#include <osg/Object>
#include <osg/Group>
#include <osg/Geode>
#include <osg/CopyOp>

class Vorton: public osgParticle::Operator {
public:
	Vorton() : osgParticle::Operator(),
		mPosition(0.f, 0.f, 1.f), mVorticity(1.f),
		mDeltaAzimuth(glm::radians(90.f)),
		mDeltaElevation(glm::radians(90.f))
	{}

	Vorton(const Vorton & other, const osg::CopyOp & copyOp = osg::CopyOp::SHALLOW_COPY) :
		osgParticle::Operator(other, copyOp), 
		mPosition(other.mPosition), mVorticity(other.mVorticity),
		mDeltaAzimuth(other.mDeltaAzimuth), mDeltaElevation(other.mDeltaElevation)
	{}

	Vorton(glm::vec3 & pos, float azDelta = glm::radians(90.f), float elevDelta = glm::radians(90.f)) :
		mPosition(pos), mVorticity(1.f),
		mDeltaAzimuth(azDelta),
		mDeltaElevation(elevDelta)
	{}

	void beginOperate(osgParticle::Program * prog) override {}
	void operate(osgParticle::Particle * particle, double dt) override {}

	META_Object(osgParticle, Vorton);

	virtual ~Vorton() {}

	glm::vec3 &         GetPosition() { return mPosition; }
	const glm::vec3 &   GetPosition() const { return mPosition; }
	float &             GetVorticity() { return mVorticity; }
	const float &       GetVorticity() const { return mVorticity; }

	void SetPosition(const glm::vec3 & pos) { mPosition = pos; }
	void SetVorticity(const float & vort) { mVorticity = vort; }

	/**
	 \brief Samples the vortex field of this vorton and returns the induced force vector,
	 based on the vorticity of this Vorton.

	 \param sample The point at which to calculate the force, in cartesian coordinates relative to the center of the planet.

	 \return the force vector, in cartesian coordinates
	 */
	virtual glm::vec3 ComputeForceVector(const glm::vec3 & sample) const { return glm::vec3(0.f); }

	/**
	 \brief Updates the vorton.

	 It is up to derived classes to define what updating a vorton means. For example, if vortons move,
	 than this method would be used to update the position.
	 */
	virtual void Update(const float timeStep) {}
protected:
	/**
	 \brief The position of this vorton in cartesian coordinates to the center of the planet.
	*/
	glm::vec3 mPosition;

	/**
	 \brief The scalar vorticity of this Vorton.

	 The actual application of this scalar is determined by the implementing subclass.

	 \see ComputeForceVector
	*/
	float mVorticity;

	/// Bounds of influence region in azimuth. If 0, then vorton influences all points on sphere.
	/// \note specified in radians
	float mDeltaAzimuth;
	// Bounds of influence region in elevation. If 0, then vorton influences all points on sphere.
	/// \note specified in radians, 0 ≤ elevation ≤ π
	float mDeltaElevation;
};

typedef std::shared_ptr<Vorton> VortonRef;