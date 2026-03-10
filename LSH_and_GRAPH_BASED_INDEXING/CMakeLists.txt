cmake_minimum_required(VERSION 3.10)
project(VectorDB_LSH)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_EXTENSIONS OFF)

# message(STATUS "CMAKE_CURRENT_SOURCE_DIR = ${CMAKE_CURRENT_SOURCE_DIR}")
# message(STATUS "CMAKE_SOURCE_DIR = ${CMAKE_SOURCE_DIR}")

add_library(LSH_Index)
target_sources(LSH_Index
    PRIVATE
        LSH/src/LSH_Index.cpp

    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS
            "LSH/include"
            "../utils"
        FILES
            "LSH/include/LSH_Index.hpp"
)

# add_subdirectory(test)
add_subdirectory(bench)
