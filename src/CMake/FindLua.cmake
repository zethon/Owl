# Owl - www.owlclient.com
# Copyright (c) 2012-2017, Adalid Claure <aclaure@gmail.com>

SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)

if (NOT "${LUA_FOLDER}" STREQUAL "")
#    message(STATUS "Looking for specific Lua under ${LUA_FOLDER}")

    find_library(Lua_LIBRARY_RELEASE
      NAMES liblua52.a lua52.lib
      HINTS ${LUA_FOLDER}
      NO_DEFAULT_PATH
    )

    find_library(Lua_LIBRARY_DEBUG
      NAMES liblua52d.a lua52d.lib
      HINTS ${LUA_FOLDER}
      NO_DEFAULT_PATH
    )

    find_path(Lua_INCLUDE_DIR
      NAMES lua.h
      HINTS ${LUA_FOLDER}/include
      NO_DEFAULT_PATH
    )

elseif(NOT "${OWLAPI_FOLDER}" STREQUAL "")
#    message(STATUS "Looking for OWLAPI Lua under ${OWLAPI_FOLDER}")

    find_library(Lua_LIBRARY_RELEASE
      NAMES liblua52.a lua52.lib
      HINTS ${OWLAPI_FOLDER}/lib
      NO_DEFAULT_PATH
    )

    find_library(Lua_LIBRARY_DEBUG
      NAMES liblua52d.a lua52d.lib
      HINTS ${OWLAPI_FOLDER}/lib
      NO_DEFAULT_PATH
    )

    find_path(Lua_INCLUDE_DIR
      NAMES lua.h
      HINTS ${OWLAPI_FOLDER}/inc/lua
      NO_DEFAULT_PATH
    )
else()
    message(SEND_ERROR "Lua could not be found")
endif()

#if (NOT Lua_LIBRARY_DEBUG)
#    message(STATUS "No Lua DEBUG library found, using RELEASE library")
#    set(Lua_LIBRARY_DEBUG "${Lua_LIBRARY_RELEASE}")
#endif()

if (Lua_LIBRARY_RELEASE AND Lua_INCLUDE_DIR)

    if (Lua_LIBRARY_RELEASE AND Lua_LIBRARY_DEBUG)
            set(Lua_LIBRARY optimized ${Lua_LIBRARY_RELEASE} debug ${Lua_LIBRARY_DEBUG} )
    else()
            set(Lua_LIBRARY ${Lua_LIBRARY_RELEASE} )
    endif()

    message(STATUS "Lua include folder: ${Lua_INCLUDE_DIR}")
    message(STATUS "Lua libraries folder: ${Lua_LIBRARY}")
else()
	message(SEND_ERROR "Lua has NOT been found")
	set (Lua_FOUND false)
endif()

mark_as_advanced(Lua_INCLUDE_DIR)
mark_as_advanced(Lua_LIBRARY_DEBUG)
mark_as_advanced(Lua_LIBRARY_RELEASE)
