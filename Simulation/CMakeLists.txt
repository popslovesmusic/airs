# IGSOA-SIM - CLI-Only Build System
# Builds C++ core engines, CLI executable, and optional test suite

cmake_minimum_required(VERSION 3.15)
project(IGSOA_SIM VERSION 1.0.0 LANGUAGES CXX)

# ============================================================================
# BUILD CONFIGURATION
# ============================================================================

# FIX H1.1: Set default build type to Release for optimized builds
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Release' as none was specified")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build (Debug, Release, RelWithDebInfo, MinSizeRel)" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
endif()

# ============================================================================
# PROJECT OPTIONS
# ============================================================================

option(BUILD_CLI "Build CLI executable (dase_cli)" ON)
option(BUILD_ENGINE_DLLS "Build engine DLLs for external use" ON)
option(BUILD_TESTS "Build C++ unit tests" OFF)
option(USE_GTEST "Use Google Test framework for tests" ON)
option(ENABLE_AVX2 "Enable AVX2 SIMD instructions" ON)
option(ENABLE_OPENMP "Enable OpenMP parallelization" ON)

# ============================================================================
# C++ STANDARD AND COMPILER REQUIREMENTS
# ============================================================================

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Position Independent Code for shared libraries
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# ============================================================================
# FIND DEPENDENCIES
# ============================================================================

# Find OpenMP (optional but recommended)
if(ENABLE_OPENMP)
    find_package(OpenMP)
    if(OpenMP_CXX_FOUND)
        message(STATUS "OpenMP found: ${OpenMP_CXX_VERSION}")
    else()
        message(WARNING "OpenMP not found - parallel performance will be degraded")
    endif()
endif()

# FFTW3 - look in project root first
find_library(FFTW3_LIBRARY NAMES fftw3 libfftw3-3 PATHS ${CMAKE_CURRENT_SOURCE_DIR} NO_DEFAULT_PATH)
if(NOT FFTW3_LIBRARY)
    find_library(FFTW3_LIBRARY NAMES fftw3 libfftw3-3)
endif()

if(FFTW3_LIBRARY)
    message(STATUS "Found FFTW3: ${FFTW3_LIBRARY}")
else()
    message(FATAL_ERROR "FFTW3 library not found. Please place libfftw3-3.lib in project root.")
endif()

# Find FFTW3 header
find_file(FFTW3_HEADER fftw3.h PATHS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/ftt NO_DEFAULT_PATH)
if(FFTW3_HEADER)
    get_filename_component(FFTW3_INCLUDE_DIR ${FFTW3_HEADER} DIRECTORY)
    message(STATUS "Found FFTW3 header: ${FFTW3_HEADER}")
endif()

# Google Test - optional for enhanced testing
if(BUILD_TESTS AND USE_GTEST)
    find_package(GTest CONFIG)
    if(GTest_FOUND)
        message(STATUS "Found Google Test: ${GTest_VERSION}")
        include(GoogleTest)
    else()
        message(STATUS "Google Test not found - will use basic test builds")
        set(USE_GTEST OFF)
    endif()
endif()

# ============================================================================
# UNIFIED COMPILER FLAGS
# ============================================================================

set(DASE_COMPILE_FLAGS_MSVC
    /EHsc /bigobj /std:c++17 /O2 /Ob3 /Oi /Ot
    /fp:fast /GL /DNOMINMAX /openmp /MD
)

set(DASE_COMPILE_FLAGS_GCC
    -O3 -march=native -fopenmp -ffast-math -funroll-loops
)

# Select flags based on compiler
if(MSVC)
    set(DASE_COMPILE_FLAGS ${DASE_COMPILE_FLAGS_MSVC})
else()
    set(DASE_COMPILE_FLAGS ${DASE_COMPILE_FLAGS_GCC})
endif()

# ============================================================================
# CORE LIBRARY (Static)
# ============================================================================

add_library(dase_core STATIC
    src/cpp/analog_universal_node_engine_avx2.cpp
)

target_include_directories(dase_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${FFTW3_INCLUDE_DIR}
)

target_link_libraries(dase_core PUBLIC ${FFTW3_LIBRARY})

# Apply compiler optimizations
target_compile_options(dase_core PRIVATE ${DASE_COMPILE_FLAGS})

# ============================================================================
# UTILITY LIBRARY (Logger and common utilities)
# ============================================================================

add_library(igsoa_utils STATIC
    src/cpp/utils/logger.cpp
)

target_include_directories(igsoa_utils PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Apply compiler optimizations
target_compile_options(igsoa_utils PRIVATE ${DASE_COMPILE_FLAGS})

# ============================================================================
# SID-SSP LIBRARY (Semantic Inference DAG + Stability-Seeking Processor)
# ============================================================================

# Header-only library for SID-SSP (Phase 1-2 complete)
add_library(sid_ssp INTERFACE)

target_include_directories(sid_ssp INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dase_cli/src
)

# Add source files for IDE visibility (header-only, no compilation)
target_sources(sid_ssp INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp/sid_ssp/sid_semantic_processor.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp/sid_ssp/sid_mixer.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp/sid_ssp/sid_diagram.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp/sid_ssp/sid_ast.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp/sid_ssp/sid_ast_utils.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp/sid_ssp/sid_ast_to_diagram.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp/sid_ssp/sid_parser.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp/sid_ssp/sid_validator.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp/sid_ssp/sid_rewrite.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp/sid_ssp/sid_crf.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp/sid_ssp/sid_package_validator.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp/sid_ssp/sid_stability.hpp
)

message(STATUS "Configured SID-SSP library (header-only)")

add_library(sid_ssp_capi STATIC
    src/cpp/sid_ssp/sid_capi.cpp
)

target_include_directories(sid_ssp_capi PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dase_cli/src
)

target_link_libraries(sid_ssp_capi PUBLIC sid_ssp)
target_compile_options(sid_ssp_capi PRIVATE ${DASE_COMPILE_FLAGS})

# ============================================================================
# IGSOA GRAVITATIONAL WAVE ENGINE (Static Library)
# ============================================================================

add_library(igsoa_gw_core STATIC
    src/cpp/igsoa_gw_engine/core/symmetry_field.cpp
    src/cpp/igsoa_gw_engine/core/fractional_solver.cpp
    src/cpp/igsoa_gw_engine/core/projection_operators.cpp
    src/cpp/igsoa_gw_engine/core/source_manager.cpp
    src/cpp/igsoa_gw_engine/core/echo_generator.cpp
)

target_include_directories(igsoa_gw_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${FFTW3_INCLUDE_DIR}
)

target_link_libraries(igsoa_gw_core PUBLIC ${FFTW3_LIBRARY} igsoa_utils)

# Apply compiler optimizations
target_compile_options(igsoa_gw_core PRIVATE ${DASE_COMPILE_FLAGS})

# AVX2 support
if(ENABLE_AVX2)
    if(MSVC)
        target_compile_options(dase_core PRIVATE /arch:AVX2)
        target_compile_options(sid_ssp_capi PRIVATE /arch:AVX2)
    else()
        target_compile_options(dase_core PRIVATE -mavx2 -mfma)
        target_compile_options(sid_ssp_capi PRIVATE -mavx2 -mfma)
    endif()
endif()

# OpenMP support
if(ENABLE_OPENMP AND OpenMP_CXX_FOUND)
    target_link_libraries(dase_core PUBLIC OpenMP::OpenMP_CXX)
endif()

# ============================================================================
# ENGINE C API DLLS (Optional, for external use)
# ============================================================================

if(BUILD_ENGINE_DLLS)
    # Function to create DLL for each phase
    function(add_engine_dll DLL_NAME DESCRIPTION)
        add_library(${DLL_NAME} SHARED
            src/cpp/dase_capi.cpp
        )

        target_link_libraries(${DLL_NAME} PRIVATE dase_core)
        target_compile_definitions(${DLL_NAME} PRIVATE DASE_BUILD_DLL)
        target_compile_options(${DLL_NAME} PRIVATE ${DASE_COMPILE_FLAGS})

        if(ENABLE_AVX2)
            if(MSVC)
                target_compile_options(${DLL_NAME} PRIVATE /arch:AVX2)
            else()
                target_compile_options(${DLL_NAME} PRIVATE -mavx2 -mfma)
            endif()
        endif()

        # Link-time optimization
        if(MSVC)
            target_link_options(${DLL_NAME} PRIVATE /LTCG /OPT:REF /OPT:ICF)
        else()
            target_link_options(${DLL_NAME} PRIVATE -flto)
        endif()

        message(STATUS "Configured engine DLL: ${DLL_NAME} - ${DESCRIPTION}")
    endfunction()

    # Build engine DLLs (keeping phase4b as production version)
    add_engine_dll(dase_engine "Baseline version")
    add_engine_dll(dase_engine_phase4b "Production: Barrier elimination + AVX2")
endif()

# ============================================================================
# CLI EXECUTABLE
# ============================================================================

if(BUILD_CLI)
    add_subdirectory(dase_cli)
    message(STATUS "CLI executable build: ENABLED")
endif()

# ============================================================================
# TESTS
# ============================================================================

if(BUILD_TESTS)
    # GW Engine Basic Test
    add_executable(test_gw_engine_basic
        tests/test_gw_engine_basic.cpp
    )
    target_link_libraries(test_gw_engine_basic PRIVATE igsoa_gw_core)
    target_compile_options(test_gw_engine_basic PRIVATE ${DASE_COMPILE_FLAGS})

    # GW Waveform Generation Test
    add_executable(test_gw_waveform_generation
        tests/test_gw_waveform_generation.cpp
    )
    target_link_libraries(test_gw_waveform_generation PRIVATE igsoa_gw_core)
    target_compile_options(test_gw_waveform_generation PRIVATE ${DASE_COMPILE_FLAGS})

    # Echo Detection Test
    add_executable(test_echo_detection
        tests/test_echo_detection.cpp
    )
    target_link_libraries(test_echo_detection PRIVATE igsoa_gw_core)
    target_compile_options(test_echo_detection PRIVATE ${DASE_COMPILE_FLAGS})

    # Logger Test
    add_executable(test_logger
        tests/test_logger.cpp
    )
    target_link_libraries(test_logger PRIVATE igsoa_utils)
    target_compile_options(test_logger PRIVATE ${DASE_COMPILE_FLAGS})

    # SID Core Test (Phase 1-2 validation)
    add_executable(test_sid_core
        tests/test_sid_core.cpp
    )
    target_link_libraries(test_sid_core PRIVATE sid_ssp)
    target_compile_options(test_sid_core PRIVATE ${DASE_COMPILE_FLAGS})

    # SID Rewrite Tests (Phase 5)
    add_executable(test_sid_rewrite
        tests/test_sid_rewrite.cpp
    )
    target_link_libraries(test_sid_rewrite PRIVATE sid_ssp)
    target_compile_options(test_sid_rewrite PRIVATE ${DASE_COMPILE_FLAGS})

    # SID Integration Tests (Phase 5)
    add_executable(test_sid_integration
        tests/test_sid_integration.cpp
    )
    target_link_libraries(test_sid_integration PRIVATE sid_ssp)
    target_compile_options(test_sid_integration PRIVATE ${DASE_COMPILE_FLAGS})

    # SID Regression Tests (Phase 5)
    add_executable(test_sid_regression
        tests/test_sid_regression.cpp
    )
    target_link_libraries(test_sid_regression PRIVATE sid_ssp)
    target_compile_options(test_sid_regression PRIVATE ${DASE_COMPILE_FLAGS})

    # SID C API JSON Tests (Phase 5)
    add_executable(test_sid_capi_json
        tests/test_sid_capi_json.cpp
    )
    target_link_libraries(test_sid_capi_json PRIVATE sid_ssp_capi)
    target_compile_options(test_sid_capi_json PRIVATE ${DASE_COMPILE_FLAGS})

    # SID Port Tests (AST utils, CRF, stability)
    add_executable(test_sid_ports
        tests/test_sid_ports.cpp
    )
    target_link_libraries(test_sid_ports PRIVATE sid_ssp)
    target_compile_options(test_sid_ports PRIVATE ${DASE_COMPILE_FLAGS})

    # SID dump helper (Phase 5 validation)
    add_executable(sid_dump_expr
        tests/sid_dump_expr.cpp
    )
    target_link_libraries(sid_dump_expr PRIVATE sid_ssp_capi)
    target_compile_options(sid_dump_expr PRIVATE ${DASE_COMPILE_FLAGS})

    # SID Benchmarks (Phase 5)
    add_executable(benchmark_sid
        tests/benchmark_sid.cpp
    )
    target_link_libraries(benchmark_sid PRIVATE sid_ssp)
    target_compile_options(benchmark_sid PRIVATE ${DASE_COMPILE_FLAGS})

    message(STATUS "Configured test: test_gw_engine_basic")
    message(STATUS "Configured test: test_gw_waveform_generation")
    message(STATUS "Configured test: test_echo_detection")
    message(STATUS "Configured test: test_logger")
    message(STATUS "Configured test: test_sid_core (SID Phase 1-2)")
    message(STATUS "Configured test: test_sid_rewrite (SID Phase 5)")
    message(STATUS "Configured test: test_sid_integration (SID Phase 5)")
    message(STATUS "Configured test: test_sid_regression (SID Phase 5)")
    message(STATUS "Configured test: test_sid_capi_json (SID Phase 5)")
    message(STATUS "Configured test: test_sid_ports (SID ports)")
    message(STATUS "Configured test: sid_dump_expr (SID Phase 5)")
    message(STATUS "Configured test: benchmark_sid (SID Phase 5)")
endif()

# ============================================================================
# INSTALLATION
# ============================================================================

if(BUILD_ENGINE_DLLS)
    # Install DLLs to project root for easy access
    install(TARGETS dase_engine dase_engine_phase4b
            RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}
            LIBRARY DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})
endif()

# ============================================================================
# BUILD SUMMARY
# ============================================================================

message(STATUS "")
message(STATUS "========================================")
message(STATUS "IGSOA-SIM CLI Build Configuration")
message(STATUS "========================================")
message(STATUS "Build type:       ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ compiler:     ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "C++ standard:     C++${CMAKE_CXX_STANDARD}")
message(STATUS "AVX2 enabled:     ${ENABLE_AVX2}")
message(STATUS "OpenMP enabled:   ${ENABLE_OPENMP}")
message(STATUS "")
message(STATUS "Build targets:")
message(STATUS "  - dase_core (static library)")
message(STATUS "  - igsoa_utils (static library)")
message(STATUS "  - sid_ssp (header-only library)")
message(STATUS "  - sid_ssp_capi (static library)")
message(STATUS "  - igsoa_gw_core (static library)")
if(BUILD_CLI)
    message(STATUS "  - dase_cli (CLI executable)")
endif()
if(BUILD_ENGINE_DLLS)
    message(STATUS "  - Engine DLLs (2 versions)")
endif()
if(BUILD_TESTS)
    message(STATUS "  - C++ unit tests (5 tests, including SID Phase 1-2)")
endif()
message(STATUS "========================================")
message(STATUS "")
