add_executable(benchmark_lsh)
target_sources(benchmark_lsh
    PRIVATE
        benchmark_lsh.cpp
)

add_executable(benchmark_nsw)
target_sources(benchmark_nsw
    PRIVATE
        benchmark_nsw.cpp
    
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS
            "../NSW/include"                    # <-- points to NSW/ folder
            "../../utils"
        FILES
            "../NSW/include/NSW_Index.hpp"      # <-- correct path
)

target_link_libraries(benchmark_lsh
    PRIVATE
        LSH_Index
)