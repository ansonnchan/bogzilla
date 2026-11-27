add_executable(bogzilla_bench solver_bench.cpp)
target_link_libraries(bogzilla_bench PRIVATE bogzilla_core benchmark::benchmark_main)
bogzilla_warnings(bogzilla_bench)
target_compile_definitions(bogzilla_bench PRIVATE
  BOGZILLA_DICTIONARY="${PROJECT_SOURCE_DIR}/extension/data/enable1.txt"
  BOGZILLA_FIXTURES="${PROJECT_SOURCE_DIR}/tests/fixtures/wordshake_boards.txt")
