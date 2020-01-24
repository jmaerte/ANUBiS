INCLUDE(ExternalProject)

SET(DEPENDENCIES)

SET(INSTALL_DEPENDENCIES_DIR INSTALL CACHE STRING "Install directory for third party dependencies")

INCLUDE(${CMAKE_SOURCE_DIR}/cmake/external/config/ConfigBoost.cmake)
INCLUDE(${CMAKE_SOURCE_DIR}/cmake/external/ExternalBoost.cmake)
LIST(APPEND DEPENDENCIES Boost)
LIST(APPEND CMAKE_ARGS
        -DBOOST_ROOT=${CMAKE_CURRENT_BINARY_DIR}/${INSTALL_DEPENDENCIES_DIR}/src/Boost
        -DBoost_NO_SYSTEM_PATHS=ON
        -DBoost_USE_MULTITHREADED=${BOOST_CONF_MULTITHREADING}
        -DBoost_USE_STATIC_LIBS=${BOOST_CONF_LINK_STATIC}
        -DBoost_USE_STATIC_RUNTIME=${BOOST_CONF_STATIC_RUNTIME}
        -DBoost_DEBUG=ON
        )

LIST(APPEND CMAKE_ARGS -DUSE_SUPERBUILD=OFF)
message(STATUS "Target ANUBiS will be build using CMAKE_ARGS: ${CMAKE_ARGS}")

# Call our own project
ExternalProject_Add (
                     ANUBiS
                     EXPORT              ANUBiS
                     DEPENDS             ${DEPENDENCIES}
                     SOURCE_DIR          ${PROJECT_SOURCE_DIR}
                     CMAKE_ARGS          ${CMAKE_ARGS}
                     INSTALL_COMMAND     ""
                     BINARY_DIR          ${CMAKE_CURRENT_BINARY_DIR}/ANUBiS
                    )

