cmake_minimum_required(VERSION 3.16)
project(ssot_cpp_index LANGUAGES CXX)

option(ENABLE_AVX2 "Enable AVX2 optimizations" ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(ssot_indexer
    src/indexer.cpp
    src/tokenizer.cpp
)

add_executable(ssot_searcher
    src/searcher.cpp
    src/tokenizer.cpp
)

if(ENABLE_AVX2)
    if(MSVC)
        target_compile_options(ssot_indexer PRIVATE /arch:AVX2)
        target_compile_options(ssot_searcher PRIVATE /arch:AVX2)
    else()
        target_compile_options(ssot_indexer PRIVATE -mavx2)
        target_compile_options(ssot_searcher PRIVATE -mavx2)
    endif()
endif()

find_package(SQLite3 QUIET)
if(SQLite3_FOUND)
    target_link_libraries(ssot_indexer PRIVATE SQLite::SQLite3)
else()
    find_library(SQLITE3_LIB sqlite3)
    if(NOT SQLITE3_LIB)
        message(FATAL_ERROR "sqlite3 library not found. Install sqlite3 dev package or set SQLITE3_LIB.")
    endif()
    target_link_libraries(ssot_indexer PRIVATE ${SQLITE3_LIB})
endif()
