cmake_minimum_required(VERSION 3.16)
project(tiecodec LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

set(TIECODEC_CORE_SOURCES
  src/core/diagnostics.cpp
  src/core/source.cpp
  src/core/text_util.cpp
  src/core/file_search.cpp
  src/frontend/ast.cpp
  src/frontend/lexer.cpp
  src/frontend/parser.cpp
  src/analysis/symbols.cpp
  src/analysis/symbol_resolution.cpp
  src/analysis/environment_contract.cpp
  src/analysis/usage_analysis.cpp
  src/analysis/program.cpp
  src/analysis/local_use.cpp
  src/analysis/annotation_rules.cpp
  src/analysis/unused_locals.cpp
  src/analysis/compile_conditions.cpp
  src/backend/output_sink.cpp
  src/backend/backend_registry.cpp
  src/backend/java/java_names.cpp
  src/backend/java/java_type_mapper.cpp
  src/backend/java/java_emitter.cpp
  src/backend/java/java_expressions.cpp
  src/backend/java/java_async.cpp
  src/backend/java/java_const.cpp
  src/backend/java/java_lint.cpp
  src/backend/java/java_generics.cpp
  src/backend/java/java_imports.cpp
  src/backend/java/java_support.cpp
  src/backend/java/pinyin_table.cpp
  src/backend/java/line_map.cpp
  src/backend/android/android_resource_binder.cpp
  src/backend/android/android_environment_contract.cpp
  src/backend/android/android_backend.cpp
)

add_library(tiecodec_core STATIC ${TIECODEC_CORE_SOURCES})
target_compile_definitions(tiecodec_core
  PUBLIC TIECODEC_TEMPLATE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/templates")

target_include_directories(tiecodec_core
  PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src"
  PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/core"
  PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/frontend"
  PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/analysis"
  PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/backend"
  PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/backend/java"
  PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/backend/android"
  PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/cli")

add_executable(tiecodec src/cli/main.cpp src/cli/command_line.cpp src/cli/app_config.cpp src/cli/retrace.cpp)
target_link_libraries(tiecodec PRIVATE tiecodec_core)

enable_testing()

find_package(GTest 1.10 QUIET)
if(NOT GTest_FOUND)
  include(FetchContent)
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
  FetchContent_Declare(googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip)
  FetchContent_MakeAvailable(googletest)
endif()
include(GoogleTest)

add_executable(tiecodec_tests
  tests/layering_test.cpp
  tests/smoke_test.cpp)
target_link_libraries(tiecodec_tests PRIVATE tiecodec_core GTest::gtest GTest::gtest_main)

target_compile_definitions(tiecodec_tests
  PRIVATE TIECODEC_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
  PRIVATE TIECODEC_BINARY_DIR="${CMAKE_CURRENT_BINARY_DIR}")

gtest_discover_tests(tiecodec_tests
  PROPERTIES ENVIRONMENT "TIECODEC_TEST_COMPILER=$<TARGET_FILE:tiecodec>"
  DISCOVERY_MODE PRE_TEST)
