add_executable(bogzilla_tests
  board_test.cpp
  trie_test.cpp
  solver_test.cpp
  packed_trie_test.cpp
  dawg_test.cpp)

target_link_libraries(bogzilla_tests PRIVATE bogzilla_core GTest::gtest_main)
bogzilla_warnings(bogzilla_tests)

gtest_discover_tests(bogzilla_tests)
target_compile_definitions(bogzilla_tests PRIVATE
  BOGZILLA_FIXTURES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/fixtures"
  BOGZILLA_DICTIONARY="${PROJECT_SOURCE_DIR}/extension/data/enable1.txt")
