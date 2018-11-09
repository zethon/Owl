# Owl - www.owlclient.com
# Copyright 2012-2018, Adalid Claure

project(Owl)
set(OWL_VERSION "0.7.7")

cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# initialize conan libs
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(NO_OUTPUT_DIRS KEEP_RPATHS)

# Add our own custom CMake modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMake)

set(LUAPARSERS_FOLDER "" CACHE PATH "Lua Parses Folder")
mark_as_advanced(LUAPARSERS_FOLDER)

# configure optional projects
option(BUILD_GUI "Build Owl GUI (default ON)" ON)
option(BUILD_CONSOLE "Optionally build OwlConsole project (default OFF)" OFF)
option(BUILD_TEST "Optionally build OwlTests unit test project (default OFF)" OFF)

# Global definitions
add_definitions(
    -DQT_NO_KEYWORDS
    -DCURL_STATICLIB
    -DHUNSPELL_STATIC
    -DBOOST_TEST_NO_LIB
)

# Add this definition for release since we cannot rely on 
# "DEBUG" because we mostly use "RelWithDebInfo" builds
# for development purposes. Using "#ifndef RELEASE" we
# can now ifdef debugging code
string(APPEND CMAKE_CXX_FLAGS_RELEASE " -DRELEASE")
string(APPEND CMAKE_CXX_FLAGS_MINSIZEREL " -DRELEASE")
# TODO: Eventually support CMake 3.12 so we can use this
# syntax instead of the above
# add_compile_definitions(
#     $<$<CONFIG:Release>:-DRELEASE>
#     $<$<CONFIG:<MinSizeRel>:-DRELEASE>
# )

if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES Clang)

    add_compile_options(
        # remove dead code
        -ffunction-sections
        -fdata-sections

        # enforced errors
        -Werror=format
        -Werror=trigraphs
        -Werror=missing-braces
        -Werror=parentheses
        -Werror=return-type
        -Werror=sign-compare
        -Werror=unused-macros
        -Werror=unused-function
        -Werror=unused-label
        -Werror=unused-value
        -Werror=unused-variable
        -Werror=empty-body
        -Werror=reorder
        -Werror=switch
        -Werror=int-conversion
        -Werror=uninitialized
        -Werror=char-subscripts
        $<$<COMPILE_LANGUAGE:CXX>:-Werror=non-virtual-dtor>

        # disabled errors/warnings
        -Wno-error=date-time
    )
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES Clang)

    add_compile_options(
        $<$<COMPILE_LANGUAGE:CXX>:-stdlib=libc++>

        -Wno-\#pragma-messages
        -Wno-unused-local-typedef
        -Werror=tautological-compare
        -Werror=four-char-constants
        -Werror=constant-conversion
        -Werror=bool-conversion
        -Werror=enum-conversion
        -Werror=deprecated-declarations
        -Werror=return-stack-address

        $<$<COMPILE_LANGUAGE:CXX>:-Werror=overloaded-virtual>
        $<$<COMPILE_LANGUAGE:CXX>:-Werror=unused-private-field>
        $<$<COMPILE_LANGUAGE:CXX>:-Werror=mismatched-tags>
    )
endif()

if (MSVC)
    add_definitions(
        -DWIN32
        -D_WIN32
        -D_WINDOWS
        -DNOMINMAX
        -D_SCL_SECURE_NO_WARNINGS
        -DWIN32_LEAN_AND_MEAN

        # use this to supress the boost generated "warning C4996"
        # on Windows
        -D_SILENCE_FPOS_SEEKPOS_DEPRECATION_WARNING
    )

    add_compile_options(
        # treat warnings as errors
        /WX
    )
endif(MSVC)

if (APPLE)
    set(CMAKE_MACOSX_RPATH 1)
endif(APPLE)

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
find_package(Qt5WebEngine REQUIRED)
find_package(Qt5WebEngineWidgets REQUIRED)

if (APPLE)
    find_package(Qt5MacExtras REQUIRED)
endif (APPLE)

#############################################

# We need this to be able to include headers produced 
# QT tools. This line should be in each sub project
# CMakeLists.txt also
include_directories(${CMAKE_BINARY_DIR})

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if (BUILD_GUI)
    add_subdirectory(src)
elseif(BUILD_TEST)
    # NOTE: The `BUILD_GUI` flag is a bit of a hack. We only
    # implement it because because Travis-CI currently cannot
    # build Owl's GUI (see note in .travis.yml). However, 
    # Travis can still buld the unit tests. This has value, so
    # we go ahead and build/run the unit tests on Travis's 
    # Ubuntu image.
    include_directories(src)
    add_subdirectory(src/Parsers)
    add_subdirectory(src/Utils)
endif()

if (BUILD_CONSOLE)
    add_subdirectory(OwlConsole)
endif(BUILD_CONSOLE)

if (BUILD_TEST)
    enable_testing()
    add_subdirectory(tests)
endif (BUILD_TEST)
