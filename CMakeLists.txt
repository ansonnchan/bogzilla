cmake_minimum_required(VERSION 3.24)

project(bogzilla
  VERSION 0.1.0
  DESCRIPTION "Boggle-style word game solver"
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Tests are for native builds only; the WASM build just needs the core.
if(EMSCRIPTEN)
  set(BOGZILLA_TESTS_DEFAULT OFF)
else()
  set(BOGZILLA_TESTS_DEFAULT ON)
endif()
option(BOGZILLA_BUILD_TESTS "Build unit tests" ${BOGZILLA_TESTS_DEFAULT})

# Shared warning flags. Call on each target we own.
function(bogzilla_warnings target)
  target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Wshadow -Wconversion)
endfunction()

if(BOGZILLA_BUILD_TESTS)
  include(FetchContent)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP ON)
  FetchContent_MakeAvailable(googletest)

  enable_testing()
  include(GoogleTest)
endif()

add_subdirectory(core)

if(BOGZILLA_BUILD_TESTS)
  add_subdirectory(tests)
endif()
