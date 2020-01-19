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


include_directories(${BOOST_INCLUDE_DIR})
link_directories(${BOOST_LIBRARY_DIR})
add_definitions( -DBOOST_THREAD_VERSION=4 -DBOOST_FILESYSTEM_VERSION=3 )


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