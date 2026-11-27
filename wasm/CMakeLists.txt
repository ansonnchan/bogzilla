add_executable(bogzilla_wasm bindings.cpp)
target_link_libraries(bogzilla_wasm PRIVATE bogzilla_core)
bogzilla_warnings(bogzilla_wasm)

set_target_properties(bogzilla_wasm PROPERTIES OUTPUT_NAME bogzilla SUFFIX ".mjs")

target_link_options(bogzilla_wasm PRIVATE
  -lembind
  -sMODULARIZE=1
  -sEXPORT_ES6=1
  -sEXPORT_NAME=createBogzilla
  -sALLOW_MEMORY_GROWTH=1
  # Extension pages run under MV3's CSP, which forbids eval/new Function.
  -sDYNAMIC_EXECUTION=0
  # node is only for the smoke test.
  -sENVIRONMENT=web,worker,node)
