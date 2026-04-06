add_executable(cli_demo "cli_demo.cpp")
target_link_libraries(cli_demo
    PRIVATE
        Archive_lib
)

add_executable(thread_safe_vector_store_demo "thread_safe_vector_store_demo.cpp")
target_link_libraries(thread_safe_vector_store_demo
    PRIVATE
        VectorStore_parallel
)
#target_compile_options(thread_safe_vector_store_demo PRIVATE -fsanitize=address,undefined)
#target_link_options(thread_safe_vector_store_demo PRIVATE -fsanitize=address,undefined)

target_compile_options(thread_safe_vector_store_demo PRIVATE
    $<$<CONFIG:Debug>:-fsanitize=thread -O1 -g>
)

target_link_options(thread_safe_vector_store_demo PRIVATE
    $<$<CONFIG:Debug>:-fsanitize=thread>
)