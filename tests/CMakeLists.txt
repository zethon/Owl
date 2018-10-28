project(tests)

set(UTILS_TEST
    UtilsTest_Moment.cpp
    UtilsTest_OwlUtils.cpp
    UtilsTest_QSgml.cpp
    UtilsTest_StringMap.cpp
    UtilsTest_Version.cpp
    UtilsTest_WebClient.cpp
)

add_executable(TestUtils
    main.cpp
    ${UTILS_TEST}
)

target_link_libraries(TestUtils
    ${CONAN_LIBS}
    Qt5::Core
    Utils
)

add_test(NAME TestUtils
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/TestUtils
)