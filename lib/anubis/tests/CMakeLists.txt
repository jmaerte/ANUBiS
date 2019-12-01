project(anubis_tests)

add_executable(${PROJECT_NAME}
    main.cpp
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        jmaerte::anubis
)

add_test(anubis_tests ${PROJECT_NAME})