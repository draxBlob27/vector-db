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

add_executable(benchmark_brute_parallel "benchmark_brute_parallel.cpp")
target_link_libraries(benchmark_brute_parallel
    PRIVATE
        VectorStore_parallel
)
#target_compile_options(benchmark_brute_parallel PRIVATE -fsanitize=address,undefined)
#target_link_options(benchmark_brute_parallel PRIVATE -fsanitize=address,undefined)

#target_compile_options(benchmark_brute_parallel PRIVATE -fsanitize=thread)
#target_link_options(benchmark_brute_parallel PRIVATE -fsanitize=thread)