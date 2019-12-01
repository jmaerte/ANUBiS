project(anubis)

FILE(GLOB sources ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(${PROJECT_NAME} STATIC ${sources})

add_library(jmaerte::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} pthread)

target_include_directories(${PROJECT_NAME} PUBLIC include)
target_include_directories(${PROJECT_NAME} PRIVATE src)

target_compile_features(${PROJECT_NAME}
	PRIVATE
		cxx_std_11
)

add_subdirectory(tests)