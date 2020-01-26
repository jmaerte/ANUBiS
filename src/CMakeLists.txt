FIND_PACKAGE(Threads REQUIRED)

if (ARCHITECTURE STREQUAL "64")
    SET(Boost_ARCHITECTURE -x64)
else()
    SET(Boost_ARCHITECTURE -x86)
endif()

if(WIN32 AND "x${CMAKE_CXX_COMPILER_ARCHITECTURE_ID}" STREQUAL "x")
    message(WARNING "WIN32 compiler does not specify CMAKE_CXX_COMPILER_ARCHITECTURE_ID -- filling in manually")
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(CMAKE_CXX_COMPILER_ARCHITECTURE_ID "x64")
    else()
        set(CMAKE_CXX_COMPILER_ARCHITECTURE_ID "x86")
    endif()
    message(STATUS "Compiler architecture: ${CMAKE_CXX_COMPILER_ARCHITECTURE_ID}")
endif()

INCLUDE(${CMAKE_SOURCE_DIR}/cmake/external/config/ConfigBoost.cmake)
find_package (Boost ${BOOST_VERSION} EXACT REQUIRED COMPONENTS ${BOOST_LIBRARIES})

include_directories(${Boost_INCLUDE_DIRS})

MESSAGE(STATUS "Found Boost libraries ${Boost_INCLUDE_DIRS}")

ADD_DEFINITIONS(-DBOOST_THREAD_VERSION=4
                -DBOOST_THREAD_DONT_PROVIDE_FUTURE)
REMOVE_DEFINITIONS(-DBOOST_NO_EXCEPTIONS)

add_subdirectory(anubis)
add_subdirectory(exe)