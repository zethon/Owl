# Owl - www.owlclient.com
# Copyright (c) 2012-2017, Adalid Claure <aclaure@gmail.com>

SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)

find_path(HUNSPELL_INCLUDE_DIR
    NAMES hunspell.h
    HINTS ${OWLAPI_FOLDER}/inc/hunspell
    NO_DEFAULT_PATH
)
mark_as_advanced(HUNSPELL_INCLUDE_DIR)

if (WIN32)
	# Look for the library (sorted from most current/relevant entry to least).
	find_library(HUNSPELL_LIBRARY_DEBUG NAMES
		Hunspelld
        libhunspelld
		HINTS ${OWLAPI_FOLDER}/lib
	)
	mark_as_advanced(HUNSPELL_LIBRARY_DEBUG)	
endif()

# Look for the library (sorted from most current/relevant entry to least).
find_library(HUNSPELL_LIBRARY NAMES
    Hunspell
    libhunspell
    HINTS ${OWLAPI_FOLDER}/lib
)
mark_as_advanced(HUNSPELL_LIBRARY)

if(HUNSPELL_INCLUDE_DIR AND HUNSPELL_LIBRARY)

    if (HUNSPELL_LIBRARY AND HUNSPELL_LIBRARY_DEBUG)
        set(HUNSPELL_LIBRARIES optimized ${HUNSPELL_LIBRARY} debug ${HUNSPELL_LIBRARY_DEBUG} )
    else()
        set(HUNSPELL_LIBRARIES ${HUNSPELL_LIBRARY})
    endif()

    message(STATUS "HUNSPELL include folder: ${HUNSPELL_INCLUDE_DIR}")
    message(STATUS "HUNSPELL libraries folder: ${HUNSPELL_LIBRARIES}")
endif()
