#pragma once
#include <glm/vec3.hpp>

/// Global parameters controlling the simulation.

namespace params {
	/// The center of the planet in world space cartesian coordinates.
	extern glm::vec3 gPlanetCenter;

	/// The radius of the planet.
	extern float gPlanetRadius;
};