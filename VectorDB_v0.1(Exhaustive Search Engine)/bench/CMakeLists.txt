add_executable(benchmark)
target_sources(benchmark
    PRIVATE
        benchmark_brute.cpp
)

target_link_libraries(benchmark
    PRIVATE
        VectorStore_lib
)