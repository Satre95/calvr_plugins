FIND_PATH(OSGEARTH_INCLUDE_DIR osgEarth/Map
  PATHS
  $ENV{OSGEARTH_HOME}
  NO_DEFAULT_PATH
    PATH_SUFFIXES include
)

FIND_PATH(OSGEARTH_INCLUDE_DIR osgEarth/Map
  PATHS
  /usr/local/include
  /usr/include
  /sw/include # Fink
  /opt/local/include # DarwinPorts
  /opt/csw/include # Blastwave
  /opt/include
)

FIND_LIBRARY(OSGEARTH_LIBRARY 
  NAMES osgEarth
  PATHS $ENV{OSGEARTH_HOME}
    NO_DEFAULT_PATH
    PATH_SUFFIXES lib64 lib
)

IF(OSGEARTH_LIBRARY)
  SET(OSGEARTH_LIBRARIES ${OSGEARTH_LIBRARY})
ENDIF(OSGEARTH_LIBRARY)


FIND_LIBRARY(OSGEARTHUTIL_LIBRARY 
  NAMES osgEarthUtil
  PATHS $ENV{OSGEARTH_HOME}
    NO_DEFAULT_PATH
    PATH_SUFFIXES lib64 lib
)

IF(OSGEARTHUTIL_LIBRARY)
  SET(OSGEARTH_LIBRARIES ${OSGEARTHUTIL_LIBRARY} ${OSGEARTH_LIBRARIES})
ENDIF(OSGEARTHUTIL_LIBRARY)

FIND_LIBRARY(OSGEARTFEATURES_LIBRARY 
  NAMES osgEarthFeatures
  PATHS $ENV{OSGEARTH_HOME}
    NO_DEFAULT_PATH
    PATH_SUFFIXES lib64 lib
)

IF(OSGEARTHFEATURES_LIBRARY)
  SET(OSGEARTH_LIBRARIES ${OSGEARTHFEATURES_LIBRARY} ${OSGEARTH_LIBRARIES})
ENDIF(OSGEARTHFEATURES_LIBRARY)

FIND_LIBRARY(OSGEARTHSYMBOLOGY_LIBRARY 
  NAMES osgEarthSymbology
  PATHS $ENV{OSGEARTH_HOME}
    NO_DEFAULT_PATH
    PATH_SUFFIXES lib64 lib
)

IF(OSGEARTHSYMBOLOGY_LIBRARY)
  SET(OSGEARTH_LIBRARIES ${OSGEARTHSYMBOLOGY_LIBRARY} ${OSGEARTH_LIBRARIES})
ENDIF(OSGEARTHSYMBOLOGY_LIBRARY)

SET(OSGEARTH_FOUND "NO")
IF(OSGEARTH_LIBRARIES AND OSGEARTH_INCLUDE_DIR)
  SET(OSGEARTH_FOUND "YES")
ENDIF(OSGEARTH_LIBRARIES AND OSGEARTH_INCLUDE_DIR)

