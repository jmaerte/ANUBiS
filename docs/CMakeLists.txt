set(DOXYGEN_EXTRACT_ALL YES)
set(DOXYGEN_BUILTIN_STL_SUPPORT YES)

doxygen_add_docs(docsanubis
        anubis/simplicial/simplicial.hpp
        "${CMAKE_CURRENT_SOURCE_DIR}/doc.md"
        WORKING_DIRECTORY
        "${PROJECT_SOURCE_DIR}/include"
        )
