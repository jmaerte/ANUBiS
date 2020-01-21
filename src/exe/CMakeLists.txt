set(EXE_TARGET_NAME ${PROJECT_NAME}_exe)

set(${EXE_TARGET_NAME}_SRC
		main.cpp)

#link_directories(${BOOST_LIBRARY_DIR}/libboost_system.a)

add_executable(${EXE_TARGET_NAME} ${${EXE_TARGET_NAME}_SRC})

target_include_directories(${EXE_TARGET_NAME} PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
		"$<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CMAKE_INSTALL_INCLUDEDIR}>")

target_link_libraries (${EXE_TARGET_NAME} ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ANUBiS::ANUBiS)

IF(WIN32)
	target_link_libraries(${EXE_TARGET_NAME} winpthread)
ELSEIF(UNIX)
	target_link_libraries(${EXE_TARGET_NAME} pthread)
ENDIF()

install(TARGETS ${EXE_TARGET_NAME} DESTINATION bin)