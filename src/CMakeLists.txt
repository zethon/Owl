# We need this to be able to include headers produced 
# QT tools. This line should be in each sub project
# CMakeLists.txt also
include_directories(
    ${CMAKE_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(CMAKE_AUTOMOC ON)

add_subdirectory(Data)
add_subdirectory(Parsers)
add_subdirectory(Utils)

if (BUILD_GUI)
    add_subdirectory(Owl)
endif()

if (BUILD_CONSOLE)
    add_subdirectory(Console)
endif(BUILD_CONSOLE)

if (BUILD_TEST)
    add_subdirectory(Tests)
endif (BUILD_TEST)
