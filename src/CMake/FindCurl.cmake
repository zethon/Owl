# Owl - www.owlclient.com
# Copyright (c) 2012-2017, Adalid Claure <aclaure@gmail.com>

SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)

#message(STATUS "Looking for OWLAPI Curl under ${OWLAPI_FOLDER}")

find_path(CURL_INCLUDE_DIR
    NAMES curl.h
    HINTS ${OWLAPI_FOLDER}/inc/curl
    NO_DEFAULT_PATH
)
mark_as_advanced(CURL_INCLUDE_DIR)

# Look for the library (sorted from most current/relevant entry to least).
find_library(CURL_LIBRARY NAMES
    curl
  # Windows MSVC prebuilts:
    curllib
    libcurl_imp
    curllib_static
  # Windows older "Win32 - MSVC" prebuilts (libcurl.lib, e.g. libcurl-7.15.5-win32-msvc.zip):
    libcurl
    HINTS ${OWLAPI_FOLDER}/lib
)
mark_as_advanced(CURL_LIBRARY)

if (WIN32)
	# Look for the library (sorted from most current/relevant entry to least).
	find_library(CURL_LIBRARY_DEBUG NAMES
		libcurld
		HINTS ${OWLAPI_FOLDER}/lib
	)
	mark_as_advanced(CURL_LIBRARY_DEBUG)	
endif()

if(CURL_INCLUDE_DIR)
  foreach(_curl_version_header curlver.h curl.h)
    if(EXISTS "${CURL_INCLUDE_DIR}/curl/${_curl_version_header}")
      file(STRINGS "${CURL_INCLUDE_DIR}/curl/${_curl_version_header}" curl_version_str REGEX "^#define[\t ]+LIBCURL_VERSION[\t ]+\".*\"")

      string(REGEX REPLACE "^#define[\t ]+LIBCURL_VERSION[\t ]+\"([^\"]*)\".*" "\\1" CURL_VERSION_STRING "${curl_version_str}")
      unset(curl_version_str)
      break()
    endif()
  endforeach()
endif()

## handle the QUIETLY and REQUIRED arguments and set CURL_FOUND to TRUE if
## all listed variables are TRUE
#include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
#FIND_PACKAGE_HANDLE_STANDARD_ARGS(CURL
#                                  REQUIRED_VARS CURL_LIBRARY CURL_INCLUDE_DIR
#                                  VERSION_VAR CURL_VERSION_STRING)

if(CURL_INCLUDE_DIR AND CURL_LIBRARY)

  if (APPLE)
    set(CURL_LIBRARIES ${CURL_LIBRARY})

    get_filename_component(CURL_LIB_FOLDER ${CURL_LIBRARIES} DIRECTORY)
    message(STATUS "CURL has been found: ${CURL_LIB_FOLDER}")
    list(APPEND CURL_LIBRARIES
        "${CURL_LIB_FOLDER}/libcrypto.a"
        "${CURL_LIB_FOLDER}/libssl.a"
        "${CURL_LIB_FOLDER}/libz.a")

  elseif(WIN32)
    set(CURL_LIBRARIES ${CURL_LIBRARY})
    if (CURL_LIBRARY AND CURL_LIBRARY_DEBUG)
      set(CURL_LIBRARIES optimized ${CURL_LIBRARY} debug ${CURL_LIBRARY_DEBUG} )
    else()
      set(CURL_LIBRARIES ${CURL_LIBRARY})
    endif()

    list(APPEND CURL_LIBRARIES
        "Crypt32.lib"
        "Ws2_32.lib")
  endif()

  set(CURL_INCLUDE_DIRS ${CURL_INCLUDE_DIR})
  message(STATUS "Curl include folder: ${CURL_INCLUDE_DIR}")
  message(STATUS "Curl libraries folder: ${CURL_LIBRARIES}")
endif()
