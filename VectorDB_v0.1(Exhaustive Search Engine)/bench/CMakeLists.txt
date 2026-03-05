add_executable(benchmark_brute)
target_sources(benchmark_brute
    PRIVATE
        benchmark_brute.cpp
)

target_link_libraries(benchmark_brute
    PRIVATE
        VectorStore_lib
)