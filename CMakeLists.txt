cmake_minimum_required(VERSION 3.16)
project(tiecodec LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

add_library(tiecodec_core STATIC
  src/core/diagnostics.cpp
  src/core/source.cpp
  src/core/text_util.cpp
  src/core/file_search.cpp
  src/core/json_util.cpp)
target_include_directories(tiecodec_core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/core")
target_compile_definitions(tiecodec_core
  PUBLIC TIECODEC_TEMPLATE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/templates")

add_library(tiecodec_ir STATIC
  src/ir/ast.cpp
  src/ir/symbols.cpp
  src/ir/symbol_resolution.cpp
  src/ir/annotation_index.cpp
  src/ir/async_analysis.cpp
  src/ir/semantic_model.cpp
  src/ir/local_use.cpp
  src/ir/unused_locals.cpp
  src/ir/compile_conditions.cpp)
target_include_directories(tiecodec_ir PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/ir")
target_link_libraries(tiecodec_ir PUBLIC tiecodec_core)

add_library(tiecodec_frontend STATIC
  src/frontend/lexer.cpp
  src/frontend/parser.cpp)
target_include_directories(tiecodec_frontend PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/frontend")
target_link_libraries(tiecodec_frontend PUBLIC tiecodec_ir)

add_library(tiecodec_extensions STATIC
  src/extensions/annotation_registry.cpp
  src/extensions/requirement.cpp
  src/extensions/requirement_registry.cpp)
target_include_directories(tiecodec_extensions PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/extensions")
target_link_libraries(tiecodec_extensions PUBLIC tiecodec_ir)

add_library(tiecodec_annotations STATIC
  src/annotations/common/common_annotations.cpp)
target_include_directories(tiecodec_annotations
  PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/annotations/common")
target_link_libraries(tiecodec_annotations PUBLIC tiecodec_extensions)

add_library(tiecodec_analysis STATIC
  src/analysis/usage_analysis.cpp
  src/analysis/program.cpp
  src/analysis/requirement_validation.cpp
  src/analysis/annotation_validation.cpp
  src/analysis/annotation_pipeline.cpp)
target_include_directories(tiecodec_analysis PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/analysis")
target_link_libraries(tiecodec_analysis PUBLIC tiecodec_extensions tiecodec_ir)

add_library(tiecodec_targets_api STATIC
  src/targets/api/output_sink.cpp
  src/targets/api/target_registry.cpp)
target_include_directories(tiecodec_targets_api
  PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/targets/api")
target_link_libraries(tiecodec_targets_api PUBLIC tiecodec_extensions tiecodec_ir)

add_library(tiecodec_java STATIC
  src/targets/java/java_names.cpp
  src/targets/java/java_type_mapper.cpp
  src/targets/java/java_emitter.cpp
  src/targets/java/java_expressions.cpp
  src/targets/java/java_async.cpp
  src/targets/java/java_const.cpp
  src/targets/java/java_lint.cpp
  src/targets/java/java_generics.cpp
  src/targets/java/java_imports.cpp
  src/targets/java/java_support.cpp
  src/targets/java/pinyin_table.cpp
  src/targets/java/line_map.cpp
  src/targets/java/java_language_backend.cpp
  src/targets/java/annotations/java_annotations.cpp
  src/targets/java/requirements/java_requirements.cpp)
target_include_directories(tiecodec_java
  PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/targets/java"
  PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/targets/java/annotations"
  PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/targets/java/requirements")
target_link_libraries(tiecodec_java PUBLIC tiecodec_targets_api tiecodec_annotations
  tiecodec_extensions)

add_library(tiecodec_android STATIC
  src/targets/android/android_resource_binder.cpp
  src/targets/android/android_java_extension.cpp
  src/targets/android/android_platform_backend.cpp
  src/targets/android/annotations/android_annotations.cpp
  src/targets/android/requirements/android_requirements.cpp)
target_include_directories(tiecodec_android
  PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/targets/android"
  PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/targets/android/annotations"
  PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/targets/android/requirements")
target_link_libraries(tiecodec_android PUBLIC tiecodec_java tiecodec_targets_api
  tiecodec_extensions)

add_library(tiecodec_jvm STATIC
  src/targets/jvm/jvm_platform_backend.cpp
  src/targets/jvm/jvm_requirements.cpp)
target_include_directories(tiecodec_jvm
  PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/targets/jvm"
  PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/targets/jvm/requirements")
target_link_libraries(tiecodec_jvm PUBLIC tiecodec_java tiecodec_targets_api
  tiecodec_extensions)

add_library(tiecodec_builtin STATIC
  src/targets/builtin/register_extensions.cpp
  src/targets/builtin/register_targets.cpp)
target_include_directories(tiecodec_builtin
  PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/targets/builtin")
target_link_libraries(tiecodec_builtin PUBLIC tiecodec_java tiecodec_android tiecodec_jvm
  tiecodec_annotations tiecodec_extensions)

add_executable(tiecodec
  src/cli/main.cpp
  src/cli/command_line.cpp
  src/cli/app_config.cpp
  src/cli/retrace.cpp
  src/cli/ast_dump.cpp
  src/cli/target_config_assembler.cpp
  src/cli/source_loader.cpp
  src/cli/console_reporter.cpp
  src/cli/compilation_session.cpp)
target_include_directories(tiecodec PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/cli")
target_link_libraries(tiecodec PRIVATE tiecodec_builtin tiecodec_analysis tiecodec_frontend)

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
  tests/smoke_test.cpp
  tests/registry_test.cpp
  tests/jvm_smoke_test.cpp)
target_link_libraries(tiecodec_tests PRIVATE tiecodec_builtin tiecodec_analysis
  tiecodec_frontend GTest::gtest GTest::gtest_main)

target_compile_definitions(tiecodec_tests
  PRIVATE TIECODEC_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
  PRIVATE TIECODEC_BINARY_DIR="${CMAKE_CURRENT_BINARY_DIR}")

gtest_discover_tests(tiecodec_tests
  PROPERTIES ENVIRONMENT "TIECODEC_TEST_COMPILER=$<TARGET_FILE:tiecodec>"
  DISCOVERY_MODE PRE_TEST)
