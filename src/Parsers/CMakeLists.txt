# Owl - www.owlclient.com
# Copyright (c) 2012-2017, Adalid Claure <aclaure@gmail.com>

project(Parsers)

# we need this to be able to include headers produced by uic in our code
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set (SOURCE_FILES
    BBCodeParser.cpp
    Forum.cpp
    LuaParserBase.cpp
    OwlLua.cpp
    ParserBase.cpp
    ParserManager.cpp
    Tapatalk.cpp
    Xenforo.cpp
)

set (MOC_HEADERS
    BBCodeParser.h
    Forum.h
    LuaParserBase.h
    ParserBase.h
    ParserManager.h
    Tapatalk.h
    Xenforo.h
)

set (HEADER_FILES
    OwlLua.h
    ${MOC_HEADERS}
)

# required for the QObject and logger
qt5_wrap_cpp(MOC_SOURCE_FILES ${MOC_HEADERS})

add_library(Parsers STATIC
    ${SOURCE_FILES}
    ${HEADER_FILES}
    ${MOC_SOURCE_FILES}
)

target_link_libraries(Parsers
    Qt5::Concurrent
    Qt5::Core
    Qt5::Network
    Qt5::Widgets
    Qt5::Xml
)
