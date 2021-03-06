#pragma once
#include <glm/vec3.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/constants.hpp>
#define GLM_ENABLE_EXPERIMENTAL
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

inline osg::Vec3 GLM2OSG(const glm::vec3 & vec) {
	return osg::Vec3(vec.x, vec.y, vec.z);
}

inline glm::vec3 OSG2GLM(const osg::Vec3 & vec) {
	return glm::vec3(vec.x(), vec.y(), vec.z());
}

/**
 \brief Converts the given spherical coords to cartesian.

 (radius, inclination, azimuth) -> (x, y, z)

 \note assumes 0 ≤ inclination ≤ π
 \note Return value is in OSG coordinate system.
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

 \note assumes 0 ≤ inclination ≤ π
 \note assumes that incoming coordinates are in OSG's coordinate system.
 */
inline osg::Vec3 ConvertCartesianToSpherical(const osg::Vec3 & cartCoords) {
	static const glm::mat3 M_t = glm::transpose(glm::mat3(
			glm::vec3(1.f, 0.f, 0.f),
			glm::vec3(0.f, 0.f, 1.f),
			glm::vec3(0.f, -1.f, 0.f))
	);
	// Convert to OpenGL right handed coordinate system.
	auto alpha = M_t * OSG2GLM(cartCoords);

	const auto & x = alpha.x;
	const auto & y = alpha.y;
	const auto & z = alpha.z;

	float r = std::sqrt(x * x + y * y + z * z);
	return osg::Vec3(r,
		std::acos(z / r),
		std::atan(y / x)
	);
}

/**
 \brief Converts the given spherical coords to cartesian.

 (radius, inclination, azimuth) -> (x, y, z)

 \note assumes 0 ≤ inclination ≤ π
*/
inline osg::Vec3 ConvertSphericalToCartesian(const osg::Vec3 & spCoords) {
    static const glm::mat3 M_t_inverse = glm::inverse(glm::transpose(glm::mat3(
            glm::vec3(1.f, 0.f, 0.f),
            glm::vec3(0.f, 0.f, 1.f),
            glm::vec3(0.f, -1.f, 0.f))
    ));

	const auto & r = spCoords.x();            //dist
	const auto & theta = spCoords.y();        //elevation
	const auto & phi = spCoords.z();          //azimuth

    // Result in OpenGL coordinate system.
	glm::vec3 temp(
			r * std::sin(phi) * std::cos(theta),
			r * std::sin(phi) * std::sin(theta),
			r * std::cos(phi)
	);

	// Convert back to OSG coordinate system before returning.
	return GLM2OSG(M_t_inverse * temp);
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

// From boost hash_combine
template <typename T>
inline void hash_combine(std::size_t & seed, const T & val) {
	seed ^= std::hash<T>()(val) + 0x9e37779b9 + (seed << 6) + (seed >> 2);
}

namespace std {
    template <> struct hash<std::pair<int, int>> {
        typedef std::pair<int, int> argument_type;
        std::size_t operator() (argument_type const & f) const {
           std::size_t seed = 0;
           hash_combine(seed, f.first);
           hash_combine(seed, f.second);
           return seed;
        }
    };
}
