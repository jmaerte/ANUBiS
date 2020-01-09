project(anubis VERSION 1.0.0 LANGUAGES CXX)

set( boost_URL "http://sourceforge.net/projects/boost/files/boost/1.66.0/boost_1_66_0.tar.bz2" )
set( boost_SHA1 "b6b284acde2ad7ed49b44e856955d7b1ea4e9459" )
set( boost_INSTALL ${CMAKE_CURRENT_BINARY_DIR}/third_party/boost_1_66_0)
set( boost_INCLUDE_DIR ${boost_INSTALL}/include )
set( boost_LIB_DIR ${boost_INSTALL}/lib )

include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)

ExternalProject_Add( boost
		PREFIX boost
		URL ${boost_URL}
		URL_HASH SHA1=${boost_SHA1}
		BUILD_IN_SOURCE 1
		CONFIGURE_COMMAND
		./bootstrap.sh
		--with-libraries=system
		--with-libraries=iostreams
		--with-libraries=regex
		--with-libraries=thread
		--prefix=<INSTALL_DIR>
		BUILD_COMMAND
		./b2 install link=static variant=release threading=multi runtime-link=static
		INSTALL_COMMAND ""
		INSTALL_DIR ${boost_INSTALL} )

set(Boost_LIBRARIES
		${boost_LIB_DIR}/libboost_system.a
		${boost_LIB_DIR}/libboost_iostreams.a
		${boost_LIB_DIR}/libboost_thread.a
		${boost_LIB_DIR}/libboost_regex.a)
message( STATUS "Boost static libs: " ${Boost_LIBRARIES} )



FILE(GLOB sources ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(${PROJECT_NAME} STATIC src/complex.cpp
		src/data_types/lin/sparse.cpp
		src/multi_thread/rw_mutex.cpp
		src/data_types/potence/potence.cpp)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 11)

add_library(jmaerte::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
		PUBLIC
		$<INSTALL_INTERFACE:include>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/src
		)

target_compile_options(${PROJECT_NAME} PRIVATE -Werror)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_11)


target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES} pthread)



include_directories( ${boost_INCLUDE_DIR} )
add_dependencies( ${PROJECT_NAME} boost )

target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})

add_subdirectory(tests)