cmake_minimum_required(VERSION 3.16)
project(compressed_search LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ──────────────────────────────────────────────────────────────
# Build options
# ──────────────────────────────────────────────────────────────
option(CS_ENABLE_AVX2 "Enable AVX2 popcount/accel on x86" ON)
option(CS_ENABLE_NEON "Enable NEON popcount/accel on ARM" ON)
option(CS_BUILD_TESTS "Build simple tests" ON)

# Feature flags (can be overridden via -DCS_xxx=1/0)
option(CS_USE_LEARNED_OCC "Use learned PGM-based occ" OFF)
option(CS_USE_VEB_LAYOUT "Use van Emde Boas layout" ON)
option(CS_ENABLE_HUFFMAN_WAVELET "Use Huffman wavelet tree" OFF)

# ──────────────────────────────────────────────────────────────
# Compiler flags and definitions
# ──────────────────────────────────────────────────────────────
if (MSVC)
  # MSVC: /O2 for Release, /W4 warnings
  add_compile_options(/W4)
  if (CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(/O2)
    add_compile_definitions(NDEBUG)
  endif()
else()
  # GCC/Clang: -O3 for Release
  add_compile_options(-Wall -Wextra -pedantic)
  if (CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-O3)
    add_compile_definitions(NDEBUG)
  endif()
endif()

if (CS_ENABLE_AVX2 AND CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
  add_compile_definitions(CS_AVX2)
  if (NOT MSVC)
    add_compile_options(-mavx2 -mbmi2)
  else()
    add_compile_options(/arch:AVX2)
  endif()
endif()
if (CS_ENABLE_NEON AND CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|ARM64|arm64")
  add_compile_definitions(CS_NEON)
endif()

# Propagate feature flags to compile definitions
if (CS_USE_LEARNED_OCC)
  add_compile_definitions(CS_USE_LEARNED_OCC=1)
endif()
if (CS_USE_VEB_LAYOUT)
  add_compile_definitions(CS_USE_VEB_LAYOUT=1)
endif()
if (CS_ENABLE_HUFFMAN_WAVELET)
  add_compile_definitions(CS_ENABLE_HUFFMAN_WAVELET=1)
endif()

# ──────────────────────────────────────────────────────────────
# Main library
# ──────────────────────────────────────────────────────────────
add_library(cs STATIC
  src/api/fm_index.cpp
  src/core/bitvector.cpp
  src/core/bitvector_learned.cpp
  src/core/wavelet.cpp
  src/core/wavelet_learned.cpp
  src/core/ssa.cpp
  src/serialization/serialization.cpp
)
target_include_directories(cs PUBLIC src include)

add_executable(cs_build tools/build_index.cpp)
target_link_libraries(cs_build PRIVATE cs)

# Interactive build_index tool for loading custom text files
add_executable(build_index tools/build_index.cpp)
target_link_libraries(build_index PRIVATE cs)

add_executable(cs_query tools/query_cli.cpp)
target_link_libraries(cs_query PRIVATE cs)

add_executable(cs_bench tools/bench.cpp)
target_link_libraries(cs_bench PRIVATE cs)

add_executable(benchmark tools/benchmark.cpp)
target_link_libraries(benchmark PRIVATE cs)

# ──────────────────────────────────────────────────────────────
# Tests
# ──────────────────────────────────────────────────────────────
if (CS_BUILD_TESTS)
  enable_testing()

  # Original simple tests
  add_executable(cs_tests tests/simple_tests.cpp)
  target_link_libraries(cs_tests PRIVATE cs)
  add_test(NAME simple_tests COMMAND cs_tests)

  # BitVector tests (STEP 1)
  add_executable(bitvector_tests tests/bitvector_tests.cpp)
  target_link_libraries(bitvector_tests PRIVATE cs)
  add_test(NAME bitvector_tests COMMAND bitvector_tests)

  # WaveletTree tests (STEP 2)
  add_executable(wavelet_tests tests/wavelet_tests.cpp)
  target_link_libraries(wavelet_tests PRIVATE cs)
  add_test(NAME wavelet_tests COMMAND wavelet_tests)

  # FM-Index search tests (STEP 3)
  add_executable(fm_search_tests tests/fm_search_tests.cpp)
  target_link_libraries(fm_search_tests PRIVATE cs)
  add_test(NAME fm_search_tests COMMAND fm_search_tests)

  # Learned occ tests (STEP 4)
  add_executable(learned_occ_tests tests/learned_occ_tests.cpp)
  target_link_libraries(learned_occ_tests PRIVATE cs)
  add_test(NAME learned_occ_tests COMMAND learned_occ_tests)

  # vEB layout tests (STEP 5)
  add_executable(veb_layout_tests tests/veb_layout_tests.cpp)
  target_link_libraries(veb_layout_tests PRIVATE cs)
  add_test(NAME veb_layout_tests COMMAND veb_layout_tests)

  # Serialization tests (STEP 6)
  add_executable(serialization_tests tests/serialization_tests.cpp)
  target_link_libraries(serialization_tests PRIVATE cs)
  add_test(NAME serialization_tests COMMAND serialization_tests)

  # Simple serial test (debug)
  add_executable(simple_serial_test tests/simple_serial_test.cpp)
  target_link_libraries(simple_serial_test PRIVATE cs)

  # Debug FM (temporary)
  add_executable(debug_fm tests/debug_fm.cpp)
  target_link_libraries(debug_fm PRIVATE cs)
endif()
