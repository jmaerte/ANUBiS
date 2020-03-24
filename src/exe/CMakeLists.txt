set(EXE_TARGET_NAME ${PROJECT_NAME}_exe)

set(${EXE_TARGET_NAME}_SRC
		main.cpp)

#link_directories(${BOOST_LIBRARY_DIR}/libboost_system.a)

add_executable(${EXE_TARGET_NAME} ${${EXE_TARGET_NAME}_SRC})

if (ARCHITECTURE STREQUAL "32")
	set_target_properties(${EXE_TARGET_NAME} PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
endif()

target_include_directories(${EXE_TARGET_NAME} PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
													 "$<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CMAKE_INSTALL_INCLUDEDIR}>")

target_link_libraries (${EXE_TARGET_NAME} ANUBiS::ARITHMETIC ANUBiS::ANUBiS)

IF(WIN32)
	target_link_libraries(${EXE_TARGET_NAME} winpthread)
ELSEIF(UNIX)
	target_link_libraries(${EXE_TARGET_NAME} pthread)
ENDIF()

install(TARGETS ${EXE_TARGET_NAME} DESTINATION bin)