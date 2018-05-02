#pragma once
#include <glm/vec3.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/norm.hpp>
#include <osg/Vec3>
#include <random>
#include <ctime>
#include <cmath>

/// A set of helper math and utility functions.

/*!
\brief Fast reciprocal square root

\note This assumes "float" uses IEEE 754 format.

\see Paul Hsieh's Square Root page: http://www.azillionmonkeys.com/qed/sqroot.html

\see Charles McEniry (2007): The mathematics behind the fast inverse square root function code

\see Chris Lomont: Fast inverse square root
*/
inline float finvsqrtf(const float & val) {
	long    i = (long&)val;             // Exploit IEEE 754 inner workings.
	i = 0x5f3759df - (i >> 1);          // From Taylor's theorem and IEEE 754 format.
	float   y = (float&)i;              // Estimate of 1/sqrt(val) close enough for convergence using Newton's method.
	static const float  f = 1.5f;        // Derived from Newton's method.
	const float         x = val * 0.5f;  // Derived from Newton's method.
	y = y * (f - (x * y * y));        // Newton's method for 1/sqrt(val)
	y = y * (f - (x * y * y));        // Another iteration of Newton's method
	return y;
}

/*!
\brief Fast square root

This computes val/sqrt(val) (which is sqrt(val)) so uses the 1/sqrt formula of finvsqrtf.

\note This assumes "float" uses IEEE 754 format.

\see Paul Hsieh's Square Root page: http://www.azillionmonkeys.com/qed/sqroot.html

\see Charles McEniry (2007): The mathematics behind the fast inverse square root function code

\see Chris Lomont: Fast inverse square root
*/
inline float fsqrtf(const float & val)
{
	long    i = (long&)val;             // Exploit IEEE 754 inner workings.
	i = 0x5f3759df - (i >> 1);          // From Taylor's theorem and IEEE 754 format.
	float   y = (float&)i;              // Estimate of 1/sqrt(val) close enough for convergence using Newton's method.
	static const float  f = 1.5f;        // Derived from Newton's method.
	const float         x = val * 0.5f;  // Derived from Newton's method.
	y = y * (f - (x * y * y));        // Newton's method for 1/sqrt(val)
	y = y * (f - (x * y * y));        // Another iteration of Newton's method
	return val * y;                        // Return val / sqrt(val) which is sqrt(val)
}

/**
 \brief Converts the given spherical coords to cartesian.

 (radius, inclination, azimuth) -> (x, y, z)

 \note assumes 0 ≤ inclination ≤ π
*/
inline glm::vec3 ConvertSphericalToCartesian(const glm::vec3 & spCoords) {
	const auto & r = spCoords.x;            //dist
	const auto & theta = spCoords.y;        //elevation
	const auto & phi = spCoords.z;          //azimuth

	return glm::vec3(
		r * std::sin(phi) * std::cos(theta),
		r * std::sin(phi) * std::sin(theta),
		r * std::cos(phi)
	);
}

/**
 \brief Converts the given cartesian coords to spherical.

 (x, y, z) -> (radius, inclination, azimuth)

 \note assumes 0 ≤ elevation ≤ π
 */
inline glm::vec3 ConvertCartesianToSpherical(const glm::vec3 & cartCoords) {
	const auto & x = cartCoords.x;
	const auto & y = cartCoords.y;
	const auto & z = cartCoords.z;

	float r = std::sqrt(x * x + y * y + z * z);
	return glm::vec3(r,
		std::acos(z / r),
		std::atan(y / x)
	);
}

/// Computes the midpoint between two cartesian points.
/// \see Wolfram Sphere Point Picking http://mathworld.wolfram.com/SpherePointPicking.html
template <class T>
inline T Midpoint(const T & a, const T & b) {
	return a + 0.5f * (b - a);
}

/// Returns a random point on a unit sphere.
inline glm::vec3 RandomPointOnSphere() {
	std::random_device rd;
	std::mt19937 generator(rd());
	std::uniform_real_distribution<float> distro(0.f, 1.f);
	float u = distro(generator);
	float v = distro(generator);
	float theta = 2.f * glm::pi<float>() * u;
	float phi = std::acos(2.f * v - 1.f);

	return ConvertSphericalToCartesian(glm::vec3(1.f, theta, phi));
}

/**
 \brief Returns a random float between [0 max)
 \param max Defaults to 1.0
*/

inline float RandomFloat(const float max = 1.f) {
	std::random_device rd;
	std::mt19937 generator(rd());
	std::uniform_real_distribution<float> distro(0.f, max);
	return distro(generator);
}

/**
 \brief Project the given vector onto the given plane.

 \param vec The vector to project
 \param norm The normal of the plane

 \note Assumes the normal is already normalized to length 1
*/
inline glm::vec3 ProjectVectorOnPlane(const glm::vec3 & vec, const glm::vec3 & norm) {
	return vec - glm::dot(vec, norm) * norm;
}

/**
 \brief Map a value from its current range to a new range.
 
 \note assumes (outputMax - outputMin) > 0 and (inputMax - inputMin) > 0
*/
template <class T>
T MapToRange(const T & val, const T & inputMin, const T & inputMax, const T & outputMin, const T & outputMax) {
    return ((val - inputMin) / (inputMax - inputMin) * (outputMax - outputMin) + outputMin);
}

inline osg::Vec3 GLM2OSG(const glm::vec3 & vec) {
	return osg::Vec3(vec.x, vec.y, vec.z);
}

inline glm::vec3 OSG2GLM(const osg::Vec3 & vec) {
	return glm::vec3(vec.x(), vec.y(), vec.z());
}
