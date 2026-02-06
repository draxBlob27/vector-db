cmake_minimum_required(VERSION 3.10)
project(VectorArchive LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_EXTENSIONS OFF)

add_library(archive_lib)
target_sources(archive_lib
    PRIVATE
        src/archive.cpp
    
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS
            include
        FILES
            include/vectorDB/archive.h
            include/vectorDB/errors.h
)


add_subdirectory(test)
add_subdirectory(examples)
