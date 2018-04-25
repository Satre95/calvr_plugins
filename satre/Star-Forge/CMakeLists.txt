set(LIB_NAME Star-Forge)
set(PLUGIN_HEADERS 
	StarForgePlugin.hpp AttractorVorton.hpp
	GlobalParameters.hpp math_helper.hpp
	Planet.hpp RepulsorVorton.hpp
	Tracer.hpp Vorton.hpp
	SFObject.hpp)
set(PLUGIN_SOURCES 
	StarForgePlugin.cpp AttractorVorton.cpp
	Planet.cpp RepulsorVorton.cpp 
	SFObject.cpp)

add_library(${LIB_NAME} SHARED ${PLUGIN_HEADERS} ${PLUGIN_SOURCES})
include_directories(${OSG_INCLUDE_DIR})
target_link_libraries(${LIB_NAME} ${OSG_LIBRARIES})

ADD_CALVR_LIBRARIES(${LIB_NAME})

install(TARGETS ${LIB_NAME} DESTINATION lib/plugins)