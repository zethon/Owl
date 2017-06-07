# Owl - www.owlclient.com
# Copyright (c) 2012-2017, Adalid Claure <aclaure@gmail.com>

SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)

find_path(HTMLTIDY_INCLUDE_DIR
    NAMES tidy.h
    HINTS ${OWLAPI_FOLDER}/inc/htmltidy
    NO_DEFAULT_PATH
)
mark_as_advanced(HTMLTIDY_INCLUDE_DIR)

if (WIN32)
    # Look for the library (sorted from most current/relevant entry to least).
    find_library(HTMLTIDY_LIBRARY_DEBUG NAMES
        tidysd
        HINTS ${OWLAPI_FOLDER}/lib
    )
    mark_as_advanced(HTMLTIDY_LIBRARY_DEBUG)
endif()

# Look for the library
find_library(HTMLTIDY_LIBRARY NAMES
    libtidys.a
    tidys
    HINTS ${OWLAPI_FOLDER}/lib
)
mark_as_advanced(HTMLTIDY_LIBRARY)

if(HTMLTIDY_INCLUDE_DIR AND HTMLTIDY_LIBRARY)

    if (HTMLTIDY_LIBRARY AND HTMLTIDY_LIBRARY_DEBUG)
        set(HTMLTIDY_LIBRARIES optimized ${HTMLTIDY_LIBRARY} debug ${HTMLTIDY_LIBRARY_DEBUG} )
    else()
        set(HTMLTIDY_LIBRARIES ${HTMLTIDY_LIBRARY})
    endif()

    message(STATUS "HTMLTidy include folder: ${HTMLTIDY_INCLUDE_DIR}")
    message(STATUS "HTMLTidy libraries folder: ${HTMLTIDY_LIBRARIES}")
endif()
