include(FetchContent)
FetchContent_Declare(
  googletest
  # Specify the commit you depend on and update it regularly.
  URL https://github.com/google/googletest/archive/5376968f6948923e2411081fd9372e71a59d8e77.zip
)

FetchContent_MakeAvailable(googletest)

add_executable(lsh_test)
target_sources( lsh_test
    PRIVATE
        test_LSW.cpp
)

target_link_libraries(lsh_test
    PRIVATE
        LSH_Index
    PUBLIC
        gtest_main
)

add_executable(nsw_test)
target_sources(nsw_test
    PRIVATE
        test_NSW.cpp
)

target_link_libraries(nsw_test
    PRIVATE
        NSW_Index
    PUBLIC
        gtest_main
)