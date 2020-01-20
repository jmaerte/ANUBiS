set(EXE_TARGET_NAME ${PROJECT_NAME}_exe)

set(${EXE_TARGET_NAME}_SRC
		main.cpp)

include_directories(${BOOST_INCLUDE_DIR})
link_directories(${BOOST_LIBRARY_DIR}/libboost_system.a)

add_executable(${EXE_TARGET_NAME} ${${EXE_TARGET_NAME}_SRC})

target_link_libraries(${EXE_TARGET_NAME} ${BOOST_LIBRARY_OBJECT_NAME})
target_link_libraries(${EXE_TARGET_NAME} ANUBiS::ANUBiS)

install(TARGETS ${EXE_TARGET_NAME} DESTINATION bin)