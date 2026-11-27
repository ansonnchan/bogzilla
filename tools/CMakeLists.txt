add_executable(dict_eval dict_eval.cpp)
target_link_libraries(dict_eval PRIVATE bogzilla_core)
bogzilla_warnings(dict_eval)
