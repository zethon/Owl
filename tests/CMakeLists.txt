project(tests)

if(CMAKE_COMPILER_IS_GNUCXX)

    # TODO: This required, at least on Linux, because of GNU's
    # handling of the C++ standard library with C++ and boost's
    # unit test framework. See the comment below in regards to
    # "UtilsTest_Version.cpp"
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GLIBCXX_USE_CXX11_ABI=0")
endif()

set(UTILS_TESTS
    UtilsTest_Moment.cpp
    UtilsTest_OwlUtils.cpp
    UtilsTest_QSgml.cpp
    UtilsTest_StringMap.cpp
    UtilsTest_WebClient.cpp
)

if (not CMAKE_COMPILER_IS_GNUCXX)

    # TODO: For now we will exclude this test on Linux/GCC because of the
    # ABI build error mentioned above with GCC. It would be ideal to figure 
    # out the proper fix for this (I suspect it has to do with the Conan
    # libraries and how they were built, and how everything else is being
    # built on my Ubuntu image) but for now I want to get Linux builds
    # working with gcc, so we'll work around this for now
    list(APPEND UTILS_TESTS
        UtilsTest_Version.cpp
    )
endif()

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