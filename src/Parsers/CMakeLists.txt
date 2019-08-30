project(Parsers)

# we need this to be able to include headers produced by uic in our code
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set (SOURCE_FILES
    Base64.cpp
    BBCodeParser.cpp
    Forum.cpp
    LuaParserBase.cpp
    OwlLua.cpp
    ParserBase.cpp
    ParserManager.cpp
    Tapatalk.cpp
    Xenforo.cpp
    Xenforo2.cpp
    xrvariant.cpp
    xrbase64.cpp
)

set (MOC_HEADERS
    BBCodeParser.h
    Forum.h
    LuaParserBase.h
    ParserBase.h
    ParserManager.h
    Tapatalk.h
    Xenforo.h
    Xenforo2.h
)

set (HEADER_FILES
    Base64.cpp
    OwlLua.h
    xrbase64.h
    xrvariant.h
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
    ${CONAN_LIBS}
    Qt5::Concurrent
    Qt5::Core
    Qt5::Network
    Qt5::Widgets
    Qt5::Xml
)
