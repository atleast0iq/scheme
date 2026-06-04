find_package(GTest REQUIRED)

add_executable(scheme_tests
    interpreter_test.cpp
    tokenizer_test.cpp
    parser_test.cpp
)

target_link_libraries(scheme_tests PRIVATE interpreter GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(scheme_tests)
