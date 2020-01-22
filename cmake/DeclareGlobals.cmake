if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    SET(ADR_MODEL 64)
else()
    SET(ADR_MODEL 32)
endif()

MESSAGE(STATUS "The found address model was ${ADR_MODEL}.")