add_executable(benchmark_lsh)
target_sources(benchmark_lsh
    PRIVATE
        benchmark_lsh.cpp
)

target_link_libraries(benchmark_lsh
    PRIVATE
        LSH_Index
)