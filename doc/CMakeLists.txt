cmake_minimum_required(VERSION 3.5)

set(ORIGINAL_PROJECT_NAME "ANUBiS")
project(${ORIGINAL_PROJECT_NAME}_Docs)

include(GNUInstallDirs)

find_package(Doxygen QUIET)

if(DOXYGEN_FOUND)
    # This variables are used inside the Doxyfile.in template
    set(DOXYGEN_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}" CACHE STRING "Build directory for Doxygen html output (absolute).")
    set(DOXYGEN_INSTALL_DIR "${CMAKE_CURRENT_BINARY_DIR}/doc/${ORIGINAL_PROJECT_NAME}" CACHE STRING "Installation directory for Doxygen html output (relative to CMAKE_INSTALL_PREFIX).")
    get_filename_component(ORIGINAL_PROJECT_DIR "${CMAKE_CURRENT_SOURCE_DIR}" DIRECTORY)
    set(DOXYGEN_SOURCE_DIR "${ORIGINAL_PROJECT_DIR}/src/anubis/include")
    set(DOXYGEN_DOC_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

    set(DOXYGEN_OUT_FILE ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${DOXYGEN_OUT_FILE} @ONLY)

    add_custom_target(dox ALL
            COMMAND doxygen ${DOXYGEN_OUT_FILE}
            COMMENT "Generating doxygen documentation in ${DOXYGEN_BUILD_DIR}"
            VERBATIM)

    install(DIRECTORY ${DOXYGEN_BUILD_DIR}/html
            DESTINATION ${DOXYGEN_INSTALL_DIR})
    message(STATUS "Documentation can be found in ${DOXYGEN_BUILD_DIR}")
else()
    MESSAGE(SEND_ERROR "Doxygen not found!")
endif()