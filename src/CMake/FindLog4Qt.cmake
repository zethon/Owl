# Owl - www.owlclient.com
# Copyright (c) 2012-2017, Adalid Claure <aclaure@gmail.com>

SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)

find_path(LOG4QT_INCLUDE_DIR
    NAMES log4qt.h
    HINTS ${OWLAPI_FOLDER}/inc/log4qt
    NO_DEFAULT_PATH
)
mark_as_advanced(LOG4QT_INCLUDE_DIR)

if (WIN32)
        # Look for the library (sorted from most current/relevant entry to least).
        find_library(LOG4QT_LIBRARY_DEBUG NAMES
                Log4Qtd
                HINTS ${OWLAPI_FOLDER}/lib
        )
        mark_as_advanced(LOG4QT_LIBRARY_DEBUG)
endif()

# Look for the library
find_library(LOG4QT_LIBRARY NAMES
    log4qt
#    Log4Qt
    HINTS ${OWLAPI_FOLDER}/lib
)
mark_as_advanced(LOG4QT_LIBRARY)

if(LOG4QT_INCLUDE_DIR AND LOG4QT_LIBRARY)

    if (LOG4QT_LIBRARY AND LOG4QT_LIBRARY_DEBUG)
        set(LOG4QT_LIBRARIES optimized ${LOG4QT_LIBRARY} debug ${LOG4QT_LIBRARY_DEBUG} )
    else()
        set(LOG4QT_LIBRARIES ${LOG4QT_LIBRARY})
    endif()

    message(STATUS "LOG4QT include folder: ${LOG4QT_INCLUDE_DIR}")
    message(STATUS "LOG4QT libraries folder: ${LOG4QT_LIBRARIES}")
endif()
