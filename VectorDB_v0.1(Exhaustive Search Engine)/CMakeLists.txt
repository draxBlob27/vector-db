cmake_minimum_required(VERSION 3.10)
project(VectorDB)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_EXTENSIONS OFF)

add_library(VectorStore_lib)
target_sources(VectorStore_lib
    PRIVATE
        src/VectorStore.cpp

    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS
            include
        FILES
            include/VectorStore.hpp
)

add_subdirectory(test)
add_subdirectory(bench)