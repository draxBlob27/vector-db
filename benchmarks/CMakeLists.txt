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

add_executable(benchmark_brute_threadsafe "benchmark_brute_threadsafe.cpp")
target_link_libraries(benchmark_brute_threadsafe
    PRIVATE
        VectorStore_threadsafe
        VectorStore_lib
)

# target_compile_options(thread_safe_vector_store_demo PRIVATE
#     $<$<CONFIG:Debug>:-fsanitize=thread -g>
# )

# target_link_options(thread_safe_vector_store_demo PRIVATE
#     $<$<CONFIG:Debug>:-fsanitize=thread>
# )