add_executable(cli_demo "cli_demo.cpp")
target_link_libraries(cli_demo
    PRIVATE
        Archive_lib
)
target_compile_options(cli_demo PRIVATE
    $<$<CONFIG:Debug>:-fsanitize=thread -O1 -g>
)
target_link_options(cli_demo PRIVATE
    $<$<CONFIG:Debug>:-fsanitize=thread>
)

add_executable(thread_safe_vector_store_demo "thread_safe_vector_store_demo.cpp")
target_link_libraries(thread_safe_vector_store_demo
    PRIVATE
        VectorStore_threadsafe
        VectorStore_lib
)
target_compile_options(thread_safe_vector_store_demo PRIVATE
    $<$<CONFIG:Debug>:-fsanitize=thread -O1 -g>
)
target_link_options(thread_safe_vector_store_demo PRIVATE
    $<$<CONFIG:Debug>:-fsanitize=thread>
)

add_executable(thread_safe_nsw_index_demo "thread_safe_nsw_index_demo.cpp")
target_link_libraries(thread_safe_nsw_index_demo
    PRIVATE
        NSW_Index
)
target_compile_options(thread_safe_nsw_index_demo PRIVATE
    $<$<CONFIG:Debug>:-fsanitize=thread -g>
)
target_link_options(thread_safe_nsw_index_demo PRIVATE
    $<$<CONFIG:Debug>:-fsanitize=thread>
)