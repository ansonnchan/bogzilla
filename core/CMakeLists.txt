add_library(bogzilla_core
  src/board.cpp)

target_include_directories(bogzilla_core PUBLIC include)
bogzilla_warnings(bogzilla_core)
