Project(OwlConsole)

# we need this to be able to include headers produced by uic in our code
INCLUDE_DIRECTORIES(
    libs
    ${CMAKE_CURRENT_BINARY_DIR}
)

SET (SOURCE_FILES main.cpp OwlConsole.cpp)
SET (MOC_HEADERS OwlConsole.h Terminal.h)

SET (HEADER_FILES
    Core.h
    ${MOC_HEADERS}
)

if (WIN32)
    list(APPEND SOURCE_FILES TerminalWindows.cpp)
elseif (UNIX)
    list(APPEND SOURCE_FILES TerminalUnix.cpp)
endif(WIN32)

QT5_WRAP_CPP(MOC_SOURCE_FILES ${MOC_HEADERS})

ADD_EXECUTABLE(owlc
    ${SOURCE_FILES}
    ${HEADER_FILES}
    ${MOC_SOURCE_FILES}    
)

TARGET_LINK_LIBRARIES(owlc
    ${LOG4QT_LIBRARIES}
    Parsers
    Utils
)
