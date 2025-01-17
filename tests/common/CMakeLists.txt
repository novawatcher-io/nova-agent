add_executable(test_parser parser_test.cc)
target_link_libraries(test_parser PRIVATE GTest::gtest GTest::gtest_main spdlog::spdlog absl::strings)
add_test(NAME test_parser COMMAND test_parser)

add_executable(test_file_reader file_test.cc)
target_link_libraries(test_file_reader PRIVATE GTest::gtest GTest::gtest_main spdlog::spdlog)
add_test(NAME test_file_reader COMMAND test_file_reader)
