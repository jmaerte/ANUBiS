set(BOOST_VERSION 1.72.0)

if (WIN32)
    if (MSVC_VERSION EQUAL 1500) #VS2008
        set(BOOST_WIN32_COMPILER "--toolset=msvc-9.0")
    elseif(MSVC_VERSION EQUAL 1600) #VS2010
        set(BOOST_WIN32_COMPILER "--toolset=msvc-10.0")
    elseif(MSVC_VERSION EQUAL 1700) #VS2012
        set(BOOST_WIN32_COMPILER "--toolset=msvc-11.0")
    elseif(MSVC_VERSION EQUAL 1800) #VS2013
        set(BOOST_WIN32_COMPILER "--toolset=msvc-12.0")
    elseif(MSVC_VERSION EQUAL 1900) #VS2015
        set(BOOST_WIN32_COMPILER "--toolset=msvc-14.0")
    else()
        set(BOOST_WIN32_COMPILER gcc)
    endif()

    if (MINGW)
        set(BOOST_WIN32_COMPILER gcc)
    endif()
endif()

#if (NOT ${CMAKE_BUILD_TYPE})
#    MESSAGE(FATAL_ERROR "No build type was given (-DCMAKE_BUILD_TYPE)")
#endif()
if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
    SET(BOOST_CONF_VARIANT release)
else()
    SET(BOOST_CONF_VARIANT debug)
endif()

SET(BOOST_CONF_LINK_STATIC ON)
SET(BOOST_CONF_STATIC_RUNTIME OFF)
SET(BOOST_CONF_MULTITHREADING ON)