
INCLUDE(${CMAKE_SOURCE_DIR}/cmake/external/config/ConfigBoost.cmake)
string(REPLACE "." "_" BOOST_UNDERSCORE_VERSION ${BOOST_VERSION})

IF(WIN32)
    set(BOOST_COMPRESSION "zip")
    set(BOOST_SHA256 8c20440aaba21dd963c0f7149517445f50c62ce4eb689df2b5544cc89e6e621e)
    set(BOOST_BOOTSTRAP_COMMAND ./bootstrap.bat)
    set(BOOST_B2_COMMAND b2.exe)
ELSEIF(UNIX)
    set(BOOST_COMPRESSION "tar.bz2")
    set(BOOST_SHA256 59c9b274bc451cf91a9ba1dd2c7fdcaf5d60b1b3aa83f2c9fa143417cc660722)
    set(BOOST_BOOTSTRAP_COMMAND ./bootstrap.sh)
    set(BOOST_B2_COMMAND ./b2)
ENDIF()

set(BOOST_URL "https://dl.bintray.com/boostorg/release/${BOOST_VERSION}/source/boost_${BOOST_UNDERSCORE_VERSION}.${BOOST_COMPRESSION}")

if (WIN32)
    set(BOOST_TOOLSET ${BOOST_WIN32_COMPILER})
    string(PREPEND BOOST_TOOLSET "--toolset=")
    SET(BOOST_BOOTSTRAP_COMMAND ${BOOST_BOOTSTRAP_COMMAND} ${BOOST_WIN32_COMPILER})
elseif(LINUX)
    set(BOOST_TOOLSET "--toolset=gcc")
endif()

SET(BOOST_INSTALL ${INSTALL_DEPENDENCIES_DIR})

set(BOOST_UNDERSCORE_LIBRARIES ${BOOST_LIBRARIES})
list(TRANSFORM BOOST_UNDERSCORE_LIBRARIES REPLACE "-" "_")

set(BOOST_LIBRARIES_ADD ${BOOST_UNDERSCORE_LIBRARIES})
list(TRANSFORM BOOST_LIBRARIES_ADD PREPEND "--with-")

string(REPLACE ";" " " BOOST_LIBRARIES_ADD_STR "${BOOST_LIBRARIES_ADD}")

set(BOOST_LIBRARIES_WITH ${BOOST_LIBRARIES})
list(TRANSFORM BOOST_LIBRARIES_WITH PREPEND "--with-libraries=")

## configurations
IF (${BOOST_CONF_LINK_STATIC})
    SET(BOOST_LINK static)
ELSE()
    SET(BOOST_LINK shared)
ENDIF()

IF (${BOOST_CONF_STATIC_RUNTIME})
    SET(BOOST_RUNTIME_LINK static)
ELSE()
    SET(BOOST_RUNTIME_LINK shared)
ENDIF()

IF (${BOOST_CONF_MULTITHREADING})
    SET(BOOST_THREADING multi)
ELSE()
    SET(BOOST_THREADING single)
ENDIF()

MESSAGE(STATUS "Boost Configuration: Version=${BOOST_VERSION}, Compiler=${BOOST_WIN32_COMPILER}, Threading=${BOOST_THREADING}, Link=${BOOST_LINK}, runtime-link=${BOOST_RUNTIME_LINK}")

ExternalProject_Add(
        Boost
        PREFIX 				${BOOST_INSTALL}
        URL 				${BOOST_URL}
        URL_HASH 			SHA256=${BOOST_SHA256}
        BUILD_IN_SOURCE 	1
        CONFIGURE_COMMAND 	${BOOST_BOOTSTRAP_COMMAND}
                            ${BOOST_LIBRARIES_WITH}                 # this declares the used libraries on unix
        BUILD_COMMAND 		${BOOST_B2_COMMAND}
                                ${BOOST_TOOLSET}                    # toolset to use (mingw,...)
                                -q                                  # QUIET
                                -j8                                 # 8 THREADS
#                                --abbreviate-paths                 # ON SOME SYSTEMS CMAKE CANT HANDLE LONG PATHS
                                variant=${BOOST_CONF_VARIANT}
                                --enable-pic                        # needed on windows for position independent code
                                threading=${BOOST_THREADING}                 # threading
                                link=${BOOST_LINK}                  # shared or static linking
                                runtime-link=${BOOST_RUNTIME_LINK}  # shared or static runtime linking
                                cxxflags=-fPIC
                                cflags=-fPIC
                                linkflags=-fPIC
                                address-model=${ARCHITECTURE}
                                ${BOOST_LIBRARIES_ADD}              # this declares the used libraries on windows
        INSTALL_COMMAND 	    "")


#set_property(TARGET Boost PROPERTY POSITION_INDEPENDENT_CODE TRUE)

message(STATUS "Added Target Boost.")