cmake_minimum_required(VERSION 3.16)
project(sid_ssp_demo LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Set default build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

add_executable(sid_ssp_demo
    src/main.cpp
    src/sid_semantic_processor.c
    src/sid_mixer.c
    src/sid_simd.c
)

target_include_directories(sid_ssp_demo PRIVATE src)

# Enable compiler warnings
target_compile_options(sid_ssp_demo PRIVATE
    $<$<OR:$<C_COMPILER_ID:GNU>,$<C_COMPILER_ID:Clang>>:-Wall -Wextra -Wpedantic -mavx2>
    $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-Wall -Wextra -Wpedantic -mavx2>
    $<$<C_COMPILER_ID:MSVC>:/W4 /arch:AVX2>
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /arch:AVX2>
)

# Link math library on Unix-like systems
if(UNIX)
    target_link_libraries(sid_ssp_demo PRIVATE m)
endif()
