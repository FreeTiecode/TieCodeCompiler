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

set(VALIDATION_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tools/validation")
set(BASELINE_COMPILER "${CMAKE_CURRENT_SOURCE_DIR}/../prebuilt/cli/linux/x86_64/tiec")

if(EXISTS "${VALIDATION_DIR}/check_stdlib.sh")
  add_test(NAME tiecodec_stdlib_parse
    COMMAND ${VALIDATION_DIR}/check_stdlib.sh
            $<TARGET_FILE:tiecodec>
            ${CMAKE_CURRENT_SOURCE_DIR}/../stdlibs/安卓基本库/源代码)

  add_test(NAME tiecodec_stdlib_parity
    COMMAND ${VALIDATION_DIR}/check_stdlib_parity.sh
            $<TARGET_FILE:tiecodec>
            ${BASELINE_COMPILER})

  add_test(NAME tiecodec_real_projects
    COMMAND ${VALIDATION_DIR}/check_real_projects.sh
            $<TARGET_FILE:tiecodec>
            ${BASELINE_COMPILER})

  add_test(NAME tiecodec_cli
    COMMAND ${VALIDATION_DIR}/check_cli.sh
            $<TARGET_FILE:tiecodec>
            ${BASELINE_COMPILER})

  add_test(NAME tiecodec_android_annotations
    COMMAND ${VALIDATION_DIR}/check_android_annotations.sh
            $<TARGET_FILE:tiecodec>
            ${BASELINE_COMPILER})

  add_test(NAME tiecodec_stdlib_lint
    COMMAND ${VALIDATION_DIR}/check_stdlib_lint.sh
            $<TARGET_FILE:tiecodec>
            ${BASELINE_COMPILER})

  add_test(NAME tiecodec_async
    COMMAND ${VALIDATION_DIR}/check_async.sh
            $<TARGET_FILE:tiecodec>
            ${BASELINE_COMPILER})

  add_test(NAME tiecodec_friendly_names
    COMMAND ${VALIDATION_DIR}/check_friendly_names.sh
            $<TARGET_FILE:tiecodec>
            ${BASELINE_COMPILER})

  add_test(NAME tiecodec_line_map
    COMMAND ${VALIDATION_DIR}/check_line_map.sh
            $<TARGET_FILE:tiecodec>
            ${BASELINE_COMPILER})

  add_test(NAME tiecodec_gradle
    COMMAND ${VALIDATION_DIR}/check_gradle.sh
            $<TARGET_FILE:tiecodec>
            ${BASELINE_COMPILER})

  add_test(NAME tiecodec_hard_mode
    COMMAND ${VALIDATION_DIR}/check_hard_mode.sh
            $<TARGET_FILE:tiecodec>
            ${BASELINE_COMPILER})

  add_test(NAME tiecodec_cases
    COMMAND ${VALIDATION_DIR}/run_tests.sh
            $<TARGET_FILE:tiecodec>
            ${VALIDATION_DIR}
            ${BASELINE_COMPILER})
endif()

set(SELF_TEST_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tests")
if(EXISTS "${SELF_TEST_DIR}/check_layering.sh")
  add_test(NAME tiecodec_layering
    COMMAND bash "${SELF_TEST_DIR}/check_layering.sh")
endif()
if(EXISTS "${SELF_TEST_DIR}/smoke.sh")
  add_test(NAME tiecodec_smoke
    COMMAND bash "${SELF_TEST_DIR}/smoke.sh" $<TARGET_FILE:tiecodec>)
endif()
