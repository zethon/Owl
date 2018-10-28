project(tests)

add_executable(TestUtils
    main.cpp
    UtilsTest_Version.cpp
)

target_link_libraries(TestUtils
 ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
     ${CONAN_LIBS}
    Qt5::Core
    Utils
)

add_test(NAME TestUtils
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/TestUtils
)