add_executable(benchmark_brute "benchmark_brute.cpp")
target_link_libraries(benchmark_brute
    PRIVATE
        VectorStore_lib
)

add_executable(benchmark_lsh "benchmark_lsh.cpp")
target_link_libraries(benchmark_lsh
    PRIVATE
        LSH_Index
)

add_executable(benchmark_nsw "benchmark_nsw.cpp")
target_link_libraries(benchmark_nsw
    PRIVATE
        NSW_Index
)

add_executable(benchmark_hnsw "benchmark_hnsw.cpp")
target_include_directories(benchmark_hnsw
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)