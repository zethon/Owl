# Owl - www.owlclient.com
# Copyright 2012-2019, Adalid Claure

cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
project(Owl)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

# INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
# INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

# initialize conan libs
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(NO_OUTPUT_DIRS KEEP_RPATHS)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Add our own custom CMake modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMake)

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
    -DBOOST_STACKTRACE_GNU_SOURCE_NOT_REQUIRED
    -D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
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

        # eliminate a lot of debug linking errors
        /wd4099
    )
endif(MSVC)

if (APPLE)
    set(CMAKE_MACOSX_RPATH 1)
endif(APPLE)

# #############################################
# # find QT

# find_package(Qt5Concurrent REQUIRED)
# find_package(Qt5Core REQUIRED)
# find_package(Qt5Gui REQUIRED)
# find_package(Qt5Network REQUIRED)
# find_package(Qt5Sql REQUIRED)
# find_package(Qt5UiTools REQUIRED)
# find_package(Qt5Widgets REQUIRED)
# find_package(Qt5Xml REQUIRED)
# find_package(Qt5XmlPatterns REQUIRED)
# find_package(Qt5Test REQUIRED)
# find_package(Qt5Qml REQUIRED)
# find_package(Qt5Quick REQUIRED)
# find_package(Qt5QuickWidgets REQUIRED)
# find_package(Qt5WebEngine REQUIRED)
# find_package(Qt5WebEngineWidgets REQUIRED)
#
# if (APPLE)
#     find_package(Qt5MacExtras REQUIRED)
# endif (APPLE)

# TODO: move the version information into a dedicated
# file *just* for versioning information
file(READ "src/Owl/Core.h" filecontent)
string(REGEX MATCH "#define[ \t]+OWL_VERSION_MAJOR[ \t]+([0-9]*)" _ ${filecontent})
set(OWL_VERSION_MAJOR ${CMAKE_MATCH_1})
string(STRIP ${OWL_VERSION_MAJOR} OWL_VERSION_MAJOR)
string(REGEX MATCH "#define[ \t]+OWL_VERSION_MINOR[ \t]+([0-9]*)" _ ${filecontent})
set(OWL_VERSION_MINOR ${CMAKE_MATCH_1})
string(STRIP ${OWL_VERSION_MINOR} OWL_VERSION_MINOR)
string(REGEX MATCH "#define[ \t]+OWL_VERSION_PATCH[ \t]+([0-9]*)" _ ${filecontent})
set(OWL_VERSION_PATCH ${CMAKE_MATCH_1})
string(STRIP ${OWL_VERSION_PATCH} OWL_VERSION_PATCH)
set(OWL_VERSION "${OWL_VERSION_MAJOR}.${OWL_VERSION_MINOR}.${OWL_VERSION_PATCH}.")

if (NOT "$ENV{APPVEYOR_BUILD_NUMBER}" STREQUAL "")
    set(OWL_BUILD_NUMBER "$ENV{APPVEYOR_BUILD_NUMBER}")
elseif(NOT "$ENV{TRAVIS_BUILD_NUMBER}" STREQUAL "")
    set(OWL_BUILD_NUMBER "$ENV{TRAVIS_BUILD_NUMBER}")
else()
    set(OWL_BUILD_NUMBER "0")
endif()

add_subdirectory(src)

if (BUILD_TEST)
    enable_testing()
    add_subdirectory(tests)
endif (BUILD_TEST)
