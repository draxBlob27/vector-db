cmake_minimum_required(VERSION 3.10)
project(vectorDB)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_CXX_STANDARD_EXTENSTIONS OFF)

add_compile_options(-O3 -std=c++20 -pthread -Wall -pedantic -fno-omit-frame-pointer
# -fno-exception -fno-rtti
)

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