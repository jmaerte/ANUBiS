set(LIBRARY_TARGET_NAME ${PROJECT_NAME})

set(${LIBRARY_TARGET_NAME}_SRC
		src/complex.cpp
		../data_struc/src/sparse.cpp
		src/data_types/potence/potence.cpp
		src/probabibilistic/multi-parametric.cpp)

set(${LIBRARY_TARGET_NAME}_PUBLIC_HDR
		include/ANUBiS/complex.hpp
		include/ANUBiS/complex/io.hpp
		include/ANUBiS/probabilistic/multi-parametric.hpp)

set(${LIBRARY_TARGET_NAME}_PRIVATE_HDR
		src/calc/computation.hpp
		../data_struc/include/data_types/sparse.hpp
		src/data_types/potence/potence.hpp
        ../data_struc/include/data_types/thread_pool.hpp
		src/calc/computation.hpp
		../data_struc/src/utils.hpp)

add_library(${LIBRARY_TARGET_NAME} SHARED ${${LIBRARY_TARGET_NAME}_SRC} ${${LIBRARY_TARGET_NAME}_PUBLIC_HDR} ${${LIBRARY_TARGET_NAME}_PRIVATE_HDR})
target_compile_options(${LIBRARY_TARGET_NAME} PRIVATE "SHELL:--param ggc-min-heapsize=1000000000")

if (ARCHITECTURE STREQUAL "32")
	set_target_properties(${LIBRARY_TARGET_NAME} PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
endif()

## set exports
SET(generated_headers "${CMAKE_CURRENT_BINARY_DIR}/auto")
SET(ANUBIS_EXPORT "${generated_headers}/ANUBIS_EXPORT.h")
include(GenerateExportHeader)
generate_export_header(${LIBRARY_TARGET_NAME} EXPORT_FILE_NAME ${ANUBIS_EXPORT})

add_library(${PROJECT_NAME}::${LIBRARY_TARGET_NAME} ALIAS ${LIBRARY_TARGET_NAME})

set_target_properties(${LIBRARY_TARGET_NAME} PROPERTIES VERSION       ${${PROJECT_NAME}_VERSION}
					  									PUBLIC_HEADER "${${LIBRARY_TARGET_NAME}_PUBLIC_HDR}")

target_include_directories(${LIBRARY_TARGET_NAME} PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
														 "$<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CMAKE_INSTALL_INCLUDEDIR}>"
														 "$<BUILD_INTERFACE:${generated_headers}>")

target_include_directories(${LIBRARY_TARGET_NAME} PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>")

target_link_libraries (${LIBRARY_TARGET_NAME} ${Boost_LIBRARIES}
						${CMAKE_THREAD_LIBS_INIT}
						${PROJECT_NAME}::ALGEBRA
						${PROJECT_NAME}::DATA_STRUCTURES
						${PROJECT_NAME}::ARITHMETIC
						${PROJECT_NAME}::OUTPUT)

IF(WIN32)
	target_link_libraries(${LIBRARY_TARGET_NAME} winpthread)
ELSEIF(UNIX)
	target_link_libraries(${LIBRARY_TARGET_NAME} pthread)
ENDIF()

install(TARGETS 	  ${LIBRARY_TARGET_NAME}
		EXPORT  	  ${PROJECT_NAME}
		LIBRARY       DESTINATION "${CMAKE_INSTALL_LIBDIR}"                            COMPONENT shlib
		ARCHIVE       DESTINATION "${CMAKE_INSTALL_LIBDIR}"                            COMPONENT lib
		RUNTIME       DESTINATION "${CMAKE_INSTALL_BINDIR}"                            COMPONENT bin
		PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${LIBRARY_TARGET_NAME}" COMPONENT dev)


message(STATUS "Created target ${LIBRARY_TARGET_NAME}!")