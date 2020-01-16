set(LIBRARY_TARGET_NAME ${PROJECT_NAME})

set(${LIBRARY_TARGET_NAME}_SRC
		src/complex.cpp
		src/data_types/lin/sparse.cpp
		src/multi_thread/rw_mutex.cpp
		src/data_types/potence/potence.cpp)

set(${LIBRARY_TARGET_NAME}_PUBLIC_HDR
		include/ANUBiS/complex.hpp)

set(${LIBRARY_TARGET_NAME}_PRIVATE_HDR
		src/calc/computation.hpp
		src/data_types/lazy/stream.hpp
		src/data_types/lin/sparse.hpp
		src/data_types/potence/potence.hpp
		src/multi_thread/thread_pool.hpp
		src/multi_thread/rw_mutex.hpp
		src/calc/computation.hpp
		src/calc/utils.hpp)


include(ExternalProject)

## Compile Boost dynamically (platform independent)
set(BOOST_VERSION 1.72.0)
string(REPLACE "." "_" BOOST_URL_VERSION ${BOOST_VERSION})

IF(WIN32)
	set(BOOST_COMPRESSION "zip")
	set(BOOST_SHA256 8c20440aaba21dd963c0f7149517445f50c62ce4eb689df2b5544cc89e6e621e)
	set(OS_BOOTSTRAP_COMMAND ./bootstrap.bat mingw)
ELSEIF(UNIX)
	set(BOOST_COMPRESSION "tar.bz2")
	set(BOOST_SHA256 59c9b274bc451cf91a9ba1dd2c7fdcaf5d60b1b3aa83f2c9fa143417cc660722)
	set(OS_BOOTSTRAP_COMMAND ./bootstrap.sh)
ENDIF()

set(BOOST_URL "https://dl.bintray.com/boostorg/release/${BOOST_VERSION}/source/boost_${BOOST_URL_VERSION}.${BOOST_COMPRESSION}")

set(BOOST_INSTALL ${CMAKE_CURRENT_BINARY_DIR}/third_party/boost_${BOOST_URL_VERSION})
set(BOOST_INCLUDE_DIR ${BOOST_INSTALL}/include)
set(BOOST_LIB_DIR ${BOOST_INSTALL}/lib)

set(BOOST_LIBRARIES
		system
		iostreams
		regex
		thread)

set(BOOST_LIBRARIES_ADD ${BOOST_LIBRARIES})
list(TRANSFORM BOOST_LIBRARIES_ADD PREPEND "--with-")

set(BOOST_LIBRARIES_WITH ${BOOST_LIBRARIES})
list(TRANSFORM BOOST_LIBRARIES_WITH PREPEND "--with-libraries=")

set(BOOST_LIBRARY_OBJECT_NAME ${BOOST_LIBRARIES})
list(TRANSFORM BOOST_LIBRARY_OBJECT_NAME PREPEND "${BOOST_LIB_DIR}/libboost_")
list(TRANSFORM BOOST_LIBRARY_OBJECT_NAME APPEND ".a")

IF( CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" )
	SET(BOOST_FLAGS cxxflags=-fPIC cflags=-fPIC linkflags=-fPIC)
ENDIF ()

ExternalProject_Add(		Boost
		PREFIX 				Boost
		URL 				${BOOST_URL}
		URL_HASH 			SHA256=${BOOST_SHA256}
		BUILD_IN_SOURCE 	1
		CONFIGURE_COMMAND 	${OS_BOOTSTRAP_COMMAND} -static ${BOOST_LIBRARIES_WITH} --prefix=<INSTALL_DIR>
		BUILD_COMMAND 		./b2 -q --prefix=<INSTALL_DIR> toolset=gcc -static install link=static variant=release threading=multi runtime-link=static ${BOOST_LIBRARIES_ADD} ${BOOST_FLAGS}
		INSTALL_COMMAND 	""
		INSTALL_DIR 		${BOOST_INSTALL})

set_property(TARGET Boost PROPERTY POSITION_INDEPENDENT_CODE TRUE)

include_directories(${BOOST_INCLUDE_DIR})

message(STATUS "Boost static libs: ${BOOST_LIBRARIES}")


add_library(${LIBRARY_TARGET_NAME} ${${LIBRARY_TARGET_NAME}_SRC} ${${LIBRARY_TARGET_NAME}_PUBLIC_HDR} ${${LIBRARY_TARGET_NAME}_PRIVATE_HDR})
add_dependencies(${LIBRARY_TARGET_NAME} Boost)

add_library(${PROJECT_NAME}::${LIBRARY_TARGET_NAME} ALIAS ${LIBRARY_TARGET_NAME})

set_target_properties(${LIBRARY_TARGET_NAME} PROPERTIES VERSION       ${${PROJECT_NAME}_VERSION}
					  									PUBLIC_HEADER "${${LIBRARY_TARGET_NAME}_PUBLIC_HDR}")

target_include_directories(${LIBRARY_TARGET_NAME} PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
														 "$<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CMAKE_INSTALL_INCLUDEDIR}>")

target_link_libraries(${LIBRARY_TARGET_NAME} ${BOOST_LIBRARY_OBJECT_NAME})

IF(WIN32)
	target_link_libraries(${LIBRARY_TARGET_NAME} winpthread)
ELSEIF(UNIX)
	target_link_libraries(${LIBRARY_TARGET_NAME} pthread)
ENDIF()

install(TARGETS ${LIBRARY_TARGET_NAME}
		EXPORT  ${PROJECT_NAME}
		LIBRARY       DESTINATION "${CMAKE_INSTALL_LIBDIR}"                            COMPONENT shlib
		ARCHIVE       DESTINATION "${CMAKE_INSTALL_LIBDIR}"                            COMPONENT lib
		RUNTIME       DESTINATION "${CMAKE_INSTALL_BINDIR}"                            COMPONENT bin
		PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${LIBRARY_TARGET_NAME}" COMPONENT dev)


message(STATUS "Created target ${LIBRARY_TARGET_NAME}!")