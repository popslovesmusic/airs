cmake_minimum_required(VERSION 3.15)
project(dase_cli VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============================================================================
# FIND DEPENDENCIES
# ============================================================================

# Find FFTW3 for analysis integration
# First check parent directory (D:/igsoa-sim/), then project directory
find_library(FFTW3_LIBRARY
    NAMES fftw3 libfftw3-3
    PATHS ${CMAKE_SOURCE_DIR}/.. ${CMAKE_SOURCE_DIR}
    NO_DEFAULT_PATH
)
if(NOT FFTW3_LIBRARY)
    find_library(FFTW3_LIBRARY NAMES fftw3 libfftw3-3)
endif()

if(FFTW3_LIBRARY)
    message(STATUS "Found FFTW3 for analysis: ${FFTW3_LIBRARY}")
    set(USE_FFTW3 ON)
else()
    message(WARNING "FFTW3 not found - engine FFT analysis will be disabled")
    set(USE_FFTW3 OFF)
endif()

# Find FFTW3 header
find_file(FFTW3_HEADER
    NAMES fftw3.h
    PATHS ${CMAKE_SOURCE_DIR}/.. ${CMAKE_SOURCE_DIR}
    NO_DEFAULT_PATH
)
if(FFTW3_HEADER)
    get_filename_component(FFTW3_INCLUDE_DIR ${FFTW3_HEADER} DIRECTORY)
    message(STATUS "Found FFTW3 header: ${FFTW3_HEADER}")
    message(STATUS "FFTW3 include directory: ${FFTW3_INCLUDE_DIR}")
endif()

# ============================================================================
# ANALYSIS INTEGRATION LIBRARY
# ============================================================================

add_library(analysis_integration STATIC
    src/python_bridge.cpp
    src/engine_fft_analysis.cpp
    src/analysis_router.cpp
)

target_include_directories(analysis_integration PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/cpp
)

# Link FFTW3 if available
if(FFTW3_LIBRARY)
    target_link_libraries(analysis_integration PRIVATE ${FFTW3_LIBRARY})
    if(FFTW3_INCLUDE_DIR)
        target_include_directories(analysis_integration PRIVATE ${FFTW3_INCLUDE_DIR})
    endif()
    # Define USE_FFTW3 to enable FFT code
    target_compile_definitions(analysis_integration PRIVATE USE_FFTW3)
    message(STATUS "FFTW3 analysis features ENABLED")
endif()

# Compiler optimizations for analysis
if(MSVC)
    target_compile_options(analysis_integration PRIVATE
        /EHsc
        $<$<CONFIG:Release>:/O2>
        $<$<CONFIG:RelWithDebInfo>:/O2>
        $<$<CONFIG:MinSizeRel>:/O1>
        # Avoid /O2 in Debug so it stays compatible with MSVC's default /RTC1 checks
        $<$<CONFIG:Debug>:/Od>
    )
else()
    target_compile_options(analysis_integration PRIVATE
        $<$<CONFIG:Debug>:-O0>
        $<$<NOT:$<CONFIG:Debug>>:-O3>
    )
endif()

# ============================================================================
# DASE CLI Executable
# ============================================================================

add_executable(dase_cli
    src/main.cpp
    src/command_router.cpp
    src/engine_manager.cpp
)

# Include directories
target_include_directories(dase_cli PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/cpp  # For DASE engine headers
)

# Link analysis integration library
target_link_libraries(dase_cli PRIVATE analysis_integration)

# Provide SID targets if not already defined (header-only core, imported C API)
if(NOT TARGET sid_ssp)
    add_library(sid_ssp INTERFACE)
    target_include_directories(sid_ssp INTERFACE
        ${CMAKE_SOURCE_DIR}/../src/cpp
        ${CMAKE_SOURCE_DIR}/src
    )
endif()

if(NOT TARGET sid_ssp_capi)
    find_library(SID_SSP_CAPI_LIB
        NAMES sid_ssp_capi
        PATHS
            ${CMAKE_SOURCE_DIR}/../build/Debug
            ${CMAKE_SOURCE_DIR}/../build/Release
            ${CMAKE_SOURCE_DIR}/../build
    )
    if(NOT SID_SSP_CAPI_LIB)
        message(FATAL_ERROR "sid_ssp_capi library not found; build or place it under ../build/<Config>/")
    endif()
    add_library(sid_ssp_capi STATIC IMPORTED)
    set_target_properties(sid_ssp_capi PROPERTIES IMPORTED_LOCATION ${SID_SSP_CAPI_LIB})
    target_include_directories(sid_ssp_capi INTERFACE ${CMAKE_SOURCE_DIR}/../src/cpp)
endif()

# Link SID-SSP core and C API
target_link_libraries(dase_cli PRIVATE sid_ssp sid_ssp_capi)

# Link IGSOA GW core
if(NOT TARGET igsoa_gw_core)
    find_library(IGSOA_GW_CORE_LIB
        NAMES igsoa_gw_core
        PATHS
            ${CMAKE_SOURCE_DIR}/../build/Debug
            ${CMAKE_SOURCE_DIR}/../build/Release
            ${CMAKE_SOURCE_DIR}/../build
    )
    if(NOT IGSOA_GW_CORE_LIB)
        message(FATAL_ERROR "igsoa_gw_core library not found; build or place it under ../build/<Config>/")
    endif()
    add_library(igsoa_gw_core STATIC IMPORTED)
    set_target_properties(igsoa_gw_core PROPERTIES IMPORTED_LOCATION ${IGSOA_GW_CORE_LIB})
    target_include_directories(igsoa_gw_core INTERFACE ${CMAKE_SOURCE_DIR}/../src/cpp)
endif()
target_link_libraries(dase_cli PRIVATE igsoa_gw_core)

# Logger/utility library
if(NOT TARGET igsoa_utils)
    find_library(IGSOA_UTILS_LIB
        NAMES igsoa_utils
        PATHS
            ${CMAKE_SOURCE_DIR}/../build/Debug
            ${CMAKE_SOURCE_DIR}/../build/Release
            ${CMAKE_SOURCE_DIR}/../build
    )
    if(NOT IGSOA_UTILS_LIB)
        message(FATAL_ERROR "igsoa_utils library not found; build or place it under ../build/<Config>/")
    endif()
    add_library(igsoa_utils STATIC IMPORTED)
    set_target_properties(igsoa_utils PROPERTIES IMPORTED_LOCATION ${IGSOA_UTILS_LIB})
    target_include_directories(igsoa_utils INTERFACE ${CMAKE_SOURCE_DIR}/../src/cpp)
endif()
target_link_libraries(dase_cli PRIVATE igsoa_utils)

# No additional linking required for DASE engine - we load the DLL dynamically at runtime
# This allows the CLI to work without a .lib file

# Windows: Build as console application (not GUI)
if(WIN32)
    set_target_properties(dase_cli PROPERTIES
        WIN32_EXECUTABLE FALSE
    )
endif()

# Compiler flags
if(MSVC)
    target_compile_options(dase_cli PRIVATE
        /W4        # Warning level 4
        /EHsc      # Exception handling
        /std:c++17 # C++17 standard
    )
else()
    target_compile_options(dase_cli PRIVATE
        -Wall
        -Wextra
        -std=c++17
    )
endif()

# AVX2 support
if(ENABLE_AVX2)
    if(MSVC)
        target_compile_options(dase_cli PRIVATE /arch:AVX2)
    else()
        target_compile_options(dase_cli PRIVATE -mavx2 -mfma)
    endif()
endif()

# Install
install(TARGETS dase_cli
    RUNTIME DESTINATION bin
)
