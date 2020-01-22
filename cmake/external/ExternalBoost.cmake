## Compile Boost dynamically (platform independent)
set(BOOST_VERSION 1.72.0)

## gets passed by script
#set(BOOST_WIN32_COMPILER gcc) # using MinGW

#if (Boost_FOUND)
#    set(BOOST_INCLUDE_DIR ${Boost_INCLUDE_DIRS})
#    set(BOOST_LIBRARY_DIR ${Boost_LIBRARY_DIRS})
#    message(STATUS "Found Boost static libs; version ${Boost_VERSION}")
#else()
string(REPLACE "." "_" BOOST_UNDERSCORE_VERSION ${BOOST_VERSION})

#set(BOOST_WIN32_COMPILER mingw)

IF(WIN32)
    set(BOOST_COMPRESSION "zip")
    set(BOOST_SHA256 8c20440aaba21dd963c0f7149517445f50c62ce4eb689df2b5544cc89e6e621e)
    set(BOOST_BOOTSTRAP_COMMAND ./bootstrap.bat ${BOOST_WIN32_COMPILER})
    set(BOOST_B2_COMMAND b2.exe)
ELSEIF(UNIX)
    set(BOOST_COMPRESSION "tar.bz2")
    set(BOOST_SHA256 59c9b274bc451cf91a9ba1dd2c7fdcaf5d60b1b3aa83f2c9fa143417cc660722)
    set(BOOST_BOOTSTRAP_COMMAND ./bootstrap.sh)
    set(BOOST_B2_COMMAND ./b2)
ENDIF()

set(BOOST_URL "https://dl.bintray.com/boostorg/release/${BOOST_VERSION}/source/boost_${BOOST_UNDERSCORE_VERSION}.${BOOST_COMPRESSION}")

if (WIN32)
    if (MSVC_VERSION EQUAL 1500) #VS2008
        set(BOOST_TOOLSET "--toolset=msvc-9.0")
    elseif(MSVC_VERSION EQUAL 1600) #VS2010
        set(BOOST_TOOLSET "--toolset=msvc-10.0")
    elseif(MSVC_VERSION EQUAL 1700) #VS2012
        set(BOOST_TOOLSET "--toolset=msvc-11.0")
    elseif(MSVC_VERSION EQUAL 1800) #VS2013
        set(BOOST_TOOLSET "--toolset=msvc-12.0")
    elseif(MSVC_VERSION EQUAL 1900) #VS2015
        set(BOOST_TOOLSET "--toolset=msvc-14.0")
    endif(MSVC_VERSION EQUAL 1500)

    if (MINGW)
        set(BOOST_TOOLSET "--toolset=gcc")
        string(APPEND BOOST_BOOTSTRAP_COMMAND "--toolset=gcc")
    endif()
elseif(LINUX)
    set(BOOST_TOOLSET "--toolset=gcc")
endif()

#if(${BUILD_SHARED_LIBS} MATCHES OFF)
#    set(BUILD_LIBS "static")
#elseif(${BUILD_SHARED_LIBS} MATCHES ON)
#    set(BUILD_LIBS "shared")
#endif()

SET(BUILD_LIBS "static")

#set(BOOST_INCLUDE_DIR ${BOOST_INSTALL}/include)
#set(BOOST_LIB_DIR ${BOOST_INSTALL}/src/Boost/libs)

SET(BOOST_INSTALL ${INSTALL_DEPENDENCIES_DIR})

set(BOOST_UNDERSCORE_LIBRARIES ${BOOST_LIBRARIES})
list(TRANSFORM BOOST_UNDERSCORE_LIBRARIES REPLACE "-" "_")

set(BOOST_LIBRARIES_ADD ${BOOST_UNDERSCORE_LIBRARIES})
list(TRANSFORM BOOST_LIBRARIES_ADD PREPEND "--with-")

string(REPLACE ";" " " BOOST_LIBRARIES_ADD_STR "${BOOST_LIBRARIES_ADD}")

set(BOOST_LIBRARIES_WITH ${BOOST_LIBRARIES})
list(TRANSFORM BOOST_LIBRARIES_WITH PREPEND "--with-libraries=")

#set(BOOST_LIBRARY_OBJECT_NAME ${BOOST_LIBRARIES})
#list(TRANSFORM BOOST_LIBRARY_OBJECT_NAME PREPEND "${BOOST_LIB_DIR}/libboost_")
#list(TRANSFORM BOOST_LIBRARY_OBJECT_NAME APPEND ".a")

#IF( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )
#    SET(BOOST_FLAGS "cxxflags=-fPIC cflags=-fPIC linkflags=-fPIC")
#ENDIF ()

ExternalProject_Add(		Boost
        PREFIX 				${BOOST_INSTALL}
        URL 				${BOOST_URL}
        URL_HASH 			SHA256=${BOOST_SHA256}
        BUILD_IN_SOURCE 	1
        CONFIGURE_COMMAND 	${BOOST_BOOTSTRAP_COMMAND} ${BOOST_LIBRARIES_WITH}
        BUILD_COMMAND 		${BOOST_B2_COMMAND} #install
                                ${BOOST_TOOLSET}                # toolset to use (mingw,...)
                                -q                              # QUIET
                                -j8                             # 8 THREADS
                                --abbreviate-paths              # ON SOME SYSTEMS CMAKE CANT HANDLE LONG PATHS
                                --hash                          # -"-
#                                --enable-shared                 # needed on windows to enable link=shared
                                --enable-pic                    # needed on windows for position independent code
#                                variant=release                 # build variant
                                threading=multi                 # threading
                                ${BOOST_LIBRARIES_ADD}          # library list to add from boost
#                                address-model=${TARGET_BITS}      # Address model
#                                architecture=x86
                                link=static                     # shared or static linking
                                runtime-link=shared             # shared or static runtime linking
#                                ${BOOST_FLAGS}                  # platform dependent flags like fPIC
        INSTALL_COMMAND 	"")
set_property(TARGET Boost PROPERTY POSITION_INDEPENDENT_CODE TRUE)

if( WIN32 )
    set(BOOST_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/${BOOST_INSTALL}/src/Boost)
    set(BOOST_ROOT ${CMAKE_SOURCE_DIR}/${BOOST_INSTALL}/src/Boost)
else()
    set(BOOST_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/${BOOST_INSTALL}/src/Boost)
endif()

set(BOOST_LIBRARY_DIR ${CMAKE_SOURCE_DIR}/${BOOST_INSTALL}/src/Boost/libs)

message(STATUS "Boost static libs ${BOOST_LIBRARIES} installed to ${INSTALL_DEPENDENCIES_DIR}")
message(STATUS "To import boost you must include the directory ${BOOST_INCLUDE_DIR} and link the directory ${BOOST_LIBRARY_DIR}.")
#endif(Boost_FOUND)