cmake_minimum_required(VERSION 3.10)
project(vectorDB)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_CXX_STANDARD_EXTENSTIONS OFF)

# add_compile_options(-O3 -std=c++20 -pthread -Wall -pedantic -fno-omit-frame-pointer
# -fno-exception -fno-rtti
# )
add_compile_options(-mavx2 -mfma)
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3 -Wall -Wextra -pedantic -fno-omit-frame-pointer")
set(CMAKE_CXX_FLAGS_RELEASE "-Ofast -DNDEBUG -march=native")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -fno-omit-frame-pointer -fno-inline")

add_library(Archive_lib)
target_sources(Archive_lib
    PRIVATE
        "src/archive.cpp"

    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS
            "include"
            # "include/vectorDB"
            # "include/vectorDB/utils"
        FILES
            "include/vectorDB/archive.hpp"
)

add_library(VectorStore_lib)
target_sources(VectorStore_lib
    PRIVATE
        "src/VectorStore.cpp"
    
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS
            "include"
            # "include/vectorDB"
            # "include/vectorDB/utils"
        FILES
            "include/vectorDB/VectorStore.hpp"
)

add_library(LSH_Index)
target_sources(LSH_Index
    PRIVATE
        "src/LSH_Index.cpp"
    
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS
            "include"
            # "include/vectorDB"
            # "include/vectorDB/utils"
        FILES
            "include/vectorDB/LSH_Index.hpp"
)

add_library(NSW_Index)
target_sources(NSW_Index
    PRIVATE
        "src/NSW_Index.cpp"
    
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS
            "include"
            # "include/vectorDB"
            # "include/vectorDB/utils"
        FILES
            "include/vectorDB/NSW_Index.hpp"
)

add_subdirectory(benchmarks)
add_subdirectory(tests)
add_subdirectory(examples)