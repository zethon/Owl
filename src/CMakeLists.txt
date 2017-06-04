# Owl - www.owlclient.com
# Copyright (c) 2012-2017, Adalid Claure <aclaure@gmail.com>

cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
include(CMakeForceCompiler)

project(OwlReader)
set(OWL_VERSION "0.7.7")

# Add our own custom CMake modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMake)

# configure folders
set(OWLAPI_FOLDER "" CACHE PATH "Owl API Path")

set(BOOST_FOLDER "" CACHE PATH "Boost Root Path")
mark_as_advanced(BOOST_FOLDER)

set(LUA_FOLDER "" CACHE PATH "Lua Root Path")
mark_as_advanced(LUA_FOLDER)

set(HUNSPELL_FOLDER "" CACHE PATH "Hunspell Root Path")
mark_as_advanced(HUNSPELL_FOLDER)

set(LUAPARSERS_FOLDER "" CACHE PATH "Lua Parses Folder")
mark_as_advanced(LUAPARSERS_FOLDER)

set(LOG4QtT_FOLDER "" CACHE PATH "Log4Qt Folder")
mark_as_advanced(LOG4QtT_FOLDER)

# configure optional projects
option(BUILD_TEST "Optionally build OwlTests unit test project (default OFF)" OFF)
option(BUILD_CONSOLE "Optionally build OwlConsol project (default OFF)" OFF)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

if (UNIX)
    if (APPLE)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -stdlib=libc++ -ffunction-sections -fdata-sections -ftemplate-depth-256 -Wno-inconsistent-missing-override")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -Wl,-dead_strip")
    endif (APPLE)
endif (UNIX)

if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES Clang)
    add_compile_options(
        -Werror=sign-compare
        -Werror=unused-macros
        -Werror=unused-function
        -Werror=unused-label
        -Werror=unused-value
        -Werror=unused-variable
        -Werror=empty-body
        -Werror=uninitialized
    )
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES Clang)
    add_compile_options(
        -Werror=bool-conversion
        -Werror=enum-conversion
        -Werror=tautological-compare
    )
endif()

#############################################
# FIND BOOST
if (NOT "${BOOST_FOLDER}" STREQUAL "")
    set(BOOST_ROOT ${BOOST_FOLDER})
    find_package(Boost REQUIRED)
    include_directories(${Boost_INCLUDE_DIR})
elseif (NOT "${OWLAPI_FOLDER}" STREQUAL "")
    set(BOOST_ROOT "${OWLAPI_FOLDER}/inc")
    find_package(Boost REQUIRED)
    include_directories(${Boost_INCLUDE_DIR})
else()
   message(SEND_ERROR "BOOST could not be found")
endif()
#############################################

#############################################
# find LOG4QT
find_package(Log4Qt REQUIRED)
include_directories(${LOG4QT_INCLUDE_DIR})
#############################################

#############################################
# find LUA 5.2
find_package(Lua REQUIRED)
include_directories(${Lua_INCLUDE_DIR})
#############################################

#############################################
# find CURL
find_package(Curl REQUIRED)
include_directories(${CURL_INCLUDE_DIRS})
#############################################

#############################################
# find HUNSPELL
find_package(Hunspell REQUIRED)
include_directories(${HUNSPELL_INCLUDE_DIRS})
#############################################

#############################################
# find HTMLTidy
find_package(HTMLTidy REQUIRED)
include_directories(${HTMLTIDY_INCLUDE_DIR})
#############################################

#############################################
# find QT

find_package(Qt5Concurrent REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5Sql REQUIRED)
find_package(Qt5UiTools REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Xml REQUIRED)
find_package(Qt5XmlPatterns REQUIRED)
find_package(Qt5Test REQUIRED)
find_package(Qt5Qml REQUIRED)
find_package(Qt5Quick REQUIRED)
find_package(Qt5QuickWidgets REQUIRED)

# include QT requirements
add_definitions(
	${Qt5_DEFINITIONS}
	${Qt5Concurrent_DEFINITIONS}
	${Qt5Core_DEFINITIONS}
	${Qt5Network_DEFINITIONS}
	${Qt5Sql_DEFINITIONS}
	${Qt5UiTools_DEFINITIONS}
	${Qt5Widgets_DEFINITIONS}
	${Qt5Xml_DEFINITIONS}
        ${Qt5Test_DEFINITIONS}
)

if (APPLE)
    find_package(Qt5MacExtras REQUIRED)
    add_definitions(${Qt5MacExtras_DEFINITIONS})
endif (APPLE)

# configure QT modules
set(QT_USE_QTCORE TRUE)
set(QT_USE_QTMAIN TRUE)
set(QT_USE_QTNETWORK TRUE)
set(QT_USE_QTSQL TRUE)
set(QT_USE_QTXML TRUE)
set(QT_USE_QTXMLPATTERNS TRUE)
set(QT_USE_QTUITOOLS TRUE)
set(QT_USE_QTTEST TRUE)
#############################################

# Needed for Qt to coexist with Boost
add_definitions(
    -DQT_NO_KEYWORDS
    -DWIN32_LEAN_AND_MEAN
    -DCURL_STATICLIB
)

# We need this to be able to include headers produced 
# QT tools. This line should be in each sub project
# CMakeLists.txt also
include_directories(${CMAKE_BINARY_DIR})

# this is required because log4qt has subfolders
#include_directories("${CMAKE_CURRENT_SOURCE_DIR}/log4qt")

# the folder where Owl's default parsers exists
set(OWL_PARSERS_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/../parsers")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_subdirectory(Owl)
add_subdirectory(Parsers)
add_subdirectory(Utils)

if (BUILD_CONSOLE)
    add_subdirectory(OwlConsole)
endif(BUILD_CONSOLE)


if (BUILD_TEST)
    enable_testing()
    add_subdirectory(OwlTests)
endif (BUILD_TEST)
