SET(LIB_NAME MenuBasics)
SET(PLUGIN_HEADERS
    MenuBasics.h
)
IF(ANDROID)
	ADD_LIBRARY(${LIB_NAME}
		STATIC
		${PLUGIN_HEADERS}
		MenuBasics.cpp
	)
ELSE(ANDROID)
	ADD_LIBRARY(${LIB_NAME}
		SHARED
		${PLUGIN_HEADERS}
		MenuBasics.cpp
	)
ENDIF(ANDROID)

INCLUDE_DIRECTORIES(${OSG_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(${LIB_NAME} ${OSG_LIBRARIES})

ADD_CALVR_LIBRARIES(${LIB_NAME})

IF(NOT ANDROID)
	INSTALL(TARGETS ${LIB_NAME} DESTINATION lib/plugins)
ENDIF(NOT ANDROID)
