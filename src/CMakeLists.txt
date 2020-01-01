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
    
# this will generate seperate package files for each
# executable so that we can have multiple intaller
# biniaries
foreach(_CONFIG_FILE_ 
    Owl.CPackConfig.cmake 
    OwlConsole.CPackConfig.cmake
    OwlPackage.cmake)
    
    configure_file("CPack/${_CONFIG_FILE_}.in"
		"${CMAKE_BINARY_DIR}/${_CONFIG_FILE_}"
        @ONLY)
endforeach()

set(CPACK_PACKAGE_CONTACT "Owl Support <support@owl.dog>")

include(CPack)
