project(app)

add_executable(${PROJECT_NAME}
	main.cpp
)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 11)

target_link_libraries(${PROJECT_NAME}
	PRIVATE
		jmaerte::anubis
)