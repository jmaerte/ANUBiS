if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    SET(TARGET_BITS 64)
else()
    SET(TARGET_BITS 32)
endif()

MESSAGE(STATUS "The found address model was ${TARGET_BITS}.")