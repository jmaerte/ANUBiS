set(LIBRARY_TARGET_NAME DATA_STRUCTURES)

set(${LIBRARY_TARGET_NAME}_SRC
        src/sparse.cpp)

set(${LIBRARY_TARGET_NAME}_PUBLIC_HDR
        include/data_types/stream.hpp
        include/data_types/sparse.hpp
        include/data_types/memory/pool_allocator.hpp)

set(${LIBRARY_TARGET_NAME}_PRIVATE_HDR)

add_library(${LIBRARY_TARGET_NAME} SHARED ${${LIBRARY_TARGET_NAME}_SRC} ${${LIBRARY_TARGET_NAME}_PUBLIC_HDR} ${${LIBRARY_TARGET_NAME}_PRIVATE_HDR})

if (ARCHITECTURE STREQUAL "32")
    set_target_properties(${LIBRARY_TARGET_NAME} PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
endif()

## set exports
SET(generated_headers "${CMAKE_CURRENT_BINARY_DIR}/auto")
SET(DATA_STRUCTURES_EXPORT "${generated_headers}/DATA_STRUCTURES_EXPORT.h")
include(GenerateExportHeader)
generate_export_header(${LIBRARY_TARGET_NAME} EXPORT_FILE_NAME ${DATA_STRUCTURES_EXPORT})

add_library(${PROJECT_NAME}::${LIBRARY_TARGET_NAME} ALIAS ${LIBRARY_TARGET_NAME})

set_target_properties(${LIBRARY_TARGET_NAME} PROPERTIES VERSION       ${${PROJECT_NAME}_VERSION}
                                                        PUBLIC_HEADER "${${LIBRARY_TARGET_NAME}_PUBLIC_HDR}")

target_include_directories(${LIBRARY_TARGET_NAME} PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
                                                         "$<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CMAKE_INSTALL_INCLUDEDIR}>"
                                                         "$<BUILD_INTERFACE:${generated_headers}>")

target_include_directories(${LIBRARY_TARGET_NAME} PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>")

install(TARGETS 	  ${LIBRARY_TARGET_NAME}
        EXPORT  	  ${PROJECT_NAME}
        LIBRARY       DESTINATION "${CMAKE_INSTALL_LIBDIR}"                            COMPONENT shlib
        ARCHIVE       DESTINATION "${CMAKE_INSTALL_LIBDIR}"                            COMPONENT lib
        RUNTIME       DESTINATION "${CMAKE_INSTALL_BINDIR}"                            COMPONENT bin
        PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${LIBRARY_TARGET_NAME}" COMPONENT dev)


message(STATUS "Created target ${PROJECT_NAME}::${LIBRARY_TARGET_NAME}!")