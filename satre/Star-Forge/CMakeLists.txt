set(LIB_NAME Star-Forge)
set(PLUGIN_HEADERS 
	StarForge.hpp AttractorVorton.hpp
	GlobalParameters.hpp math_helper.hpp
	Planet.hpp RepulsorVorton.hpp
	Tracer.hpp Vorton.hpp
	UniverseObject.hpp OSGPlanet.hpp
	SphereTangentShooter.hpp)
set(PLUGIN_SOURCES 
	StarForge.cpp AttractorVorton.cpp
	Planet.cpp RepulsorVorton.cpp 
	UniverseObject.cpp OSGPlanet.cpp
	SphereTangentShooter.cpp)

add_library(${LIB_NAME} SHARED ${PLUGIN_HEADERS} ${PLUGIN_SOURCES})
include_directories(${OSG_INCLUDE_DIR})
target_link_libraries(${LIB_NAME} ${OSG_LIBRARIES})

ADD_CALVR_LIBRARIES(${LIB_NAME})

install(TARGETS ${LIB_NAME} DESTINATION lib/plugins)