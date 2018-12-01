project(tests)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

set(UTILS_TESTS
    UtilsTest_Moment.cpp
    UtilsTest_OwlUtils.cpp
    UtilsTest_QSgml.cpp
    UtilsTest_StringMap.cpp
    UtilsTest_Version.cpp
    UtilsTest_WebClient.cpp
)

add_executable(TestUtils
    main.cpp
    ${UTILS_TESTS}
)

target_link_libraries(TestUtils
    ${CONAN_LIBS}
    Qt5::Core
    Utils
)

add_test(NAME TestUtils
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/TestUtils
)

set(PARSER_TESTS
    ParsersTest_BBCodeParser.cpp
    ParsersTest_Forum.cpp
    ParsersTest_ParserManager.cpp
    ParsersTest_Tapatalk.cpp
    ParsersTest_XenForo.cpp
)

add_executable(TestParsers
    main.cpp
    ${PARSER_TESTS}
)

target_link_libraries(TestParsers
    ${CONAN_LIBS}
    Qt5::Core
    Parsers
    Utils
)

add_test(NAME TestParsers
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/TestParsers
)

# Diasble tests using hunspell for now (which the Owl Qt libraries
# do) on Linux. See .travis.yml for details
if (NOT Linux)

    set(OWL_TESTS
        OwlTest_BoardManager.cpp
        OwlTest_Board.cpp
    )

    add_executable(TestOwl
        main.cpp
        ${OWL_TESTS}
    )

    target_link_libraries(TestOwl
        ${CONAN_LIBS}
        Qt5::Core
        Data
        Parsers
        Utils
    )

    include_directories(../src)
    include_directories(../src/Parsers)
    include_directories(../src/Utils)

    add_test(NAME TestOwl
        COMMAND ${CMAKE_CURRENT_BINARY_DIR}/TestOwl
    )
    
endif()