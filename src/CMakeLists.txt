# Owl - www.owlclient.com
# Copyright (c) 2012-2017, Adalid Claure <aclaure@gmail.com>

cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
include(CMakeForceCompiler)

project(Owl)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MACOSX_RPATH 1)

set(OWL_VERSION "0.7.7")

# Add our own custom CMake modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMake)

set(LUAPARSERS_FOLDER "" CACHE PATH "Lua Parses Folder")
mark_as_advanced(LUAPARSERS_FOLDER)

# configure optional projects
option(BUILD_GUI "Build Owl GUI (default ON)" ON)
option(BUILD_CONSOLE "Optionally build OwlConsole project (default OFF)" OFF)
option(BUILD_TEST "Optionally build OwlTests unit test project (default OFF)" OFF)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# initialize conan libs
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(NO_OUTPUT_DIRS KEEP_RPATHS)

if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES Clang)

    add_compile_options(
        # remove dead code
        -ffunction-sections
        -fdata-sections

        -Werror=format
        -Werror=trigraphs
        $<$<COMPILE_LANGUAGE:CXX>:-Werror=non-virtual-dtor>
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
    )
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES Clang)

    add_compile_options(
        $<$<COMPILE_LANGUAGE:CXX>:-stdlib=libc++>

        # Disable the pragma message statements on Clang
        # because clang always creates a warning on them.
        -Wno-\#pragma-messages

        # Boost and libc++ headers have unused local typedefs.
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

        /wd4966
    )
endif(MSVC)

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
set(QT_USE_QTWIDGETS TRUE)
#############################################

# Needed for Qt to coexist with Boost
add_definitions(
    -DQT_NO_KEYWORDS
    -DCURL_STATICLIB
    -DHUNSPELL_STATIC
)

# We need this to be able to include headers produced 
# QT tools. This line should be in each sub project
# CMakeLists.txt also
include_directories(${CMAKE_BINARY_DIR})

# the folder where Owl's default parsers exists
set(OWL_PARSERS_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/../parsers")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_subdirectory(Parsers)
add_subdirectory(Utils)

if (BUILD_GUI)
    add_subdirectory(Owl)
endif(BUILD_GUI)

if (BUILD_CONSOLE)
    add_subdirectory(OwlConsole)
endif(BUILD_CONSOLE)

if (BUILD_TEST)
    enable_testing()
    add_subdirectory(OwlTests)
endif (BUILD_TEST)
