add_executable(benchmark_lsh)
target_sources(benchmark_lsh
    PRIVATE
        benchmark_lsh.cpp
)

add_executable(benchmark_nsw)
target_sources(benchmark_nsw
    PRIVATE
        benchmark_nsw.cpp
)

target_link_libraries(benchmark_nsw
    PUBLIC
        NSW_Index
)

target_link_libraries(benchmark_lsh
    PUBLIC
        LSH_Index
)