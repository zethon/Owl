# Owl - www.owlclient.com
# Copyright (c) 2012-2017, Adalid Claure <aclaure@gmail.com>

SET(CMAKE_BUILD_TYPE Debug)
PROJECT(OwlTests)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

SET(SOURCE_FILES 
    main.cpp
    BBCodeParserTests.cpp
    OwlUtilsTests.cpp
    ParserTests.cpp
)

SET (MOC_HEADERS
    BBCodeParserTests.h
    OwlUtilsTests.h
    ParserTests.h
)

SET(HEADER_FILES
    ${MOC_HEADERS}
)

SET(DEPENDENCY_LIST
    Parsers
    Utils
)

QT5_WRAP_CPP(MOC_SOURCE_FILES ${MOC_HEADERS})

ADD_EXECUTABLE(OwlTests 
    ${SOURCE_FILES}
    ${HEADER_FILES}
    ${MOC_SOURCE_FILES}
)

TARGET_LINK_LIBRARIES(OwlTests 
    ${DEPENDENCY_LIST}
    ${Boost_LIBRARIES}
	${LOG4QT_LIBRARIES}
    ${QT_LIBRARIES}
    ${QT_QTTEST_LIBRARY}
    Qt5::Test
)

ADD_TEST(NAME OwlTests COMMAND OwlTests)
