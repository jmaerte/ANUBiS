# Boost declarations
SET(BOOST_VERSION 1.72.0)
SET(BOOST_LIBRARIES
        system
        iostreams
        regex
        chrono
        date_time
        atomic
        exception
        thread)

SET(BOOST_LIBRARY_TAGS ${BOOST_LIBRARIES})
LIST(TRANSFORM BOOST_LIBRARY_TAGS PREPEND "Boost::")