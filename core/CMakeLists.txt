add_library(bogzilla_core
  src/board.cpp
  src/trie.cpp
  src/packed_trie.cpp
  src/dawg.cpp
  src/solver.cpp)

target_include_directories(bogzilla_core PUBLIC include)
bogzilla_warnings(bogzilla_core)
