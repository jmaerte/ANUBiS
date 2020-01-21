INCLUDE(ExternalProject)

SET(DEPENDENCIES)
SET(CMAKE_ARGS)

SET(INSTALL_DEPENDENCIES_DIR INSTALL CACHE STRING "Install directory for third party dependencies")

LIST(APPEND DEPENDENCIES Boost)
INCLUDE(${CMAKE_SOURCE_DIR}/cmake/external/ExternalBoost.cmake)

list (APPEND CMAKE_ARGS
        -DBOOST_ROOT=${CMAKE_CURRENT_BINARY_DIR}/${INSTALL_DEPENDENCIES_DIR}/src/Boost
        -DBoost_NO_SYSTEM_PATHS=ON)

# Call our own project
ExternalProject_Add (ANUBiS
        DEPENDS ${DEPENDENCIES}
        SOURCE_DIR ${PROJECT_SOURCE_DIR}
        CMAKE_ARGS -DUSE_SUPERBUILD=OFF ${CMAKE_ARGS}
        INSTALL_COMMAND ""
        BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/ANUBiS)

