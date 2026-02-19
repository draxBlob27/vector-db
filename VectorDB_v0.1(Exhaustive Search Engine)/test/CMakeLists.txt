find_package(GTest REQUIRED)

add_executable(dataset_test)
target_sources(dataset_test
    PRIVATE
        test_vdb.cpp
)
target_link_libraries(dataset_test
    PRIVATE
        VectorStore_lib
        GTest::Main
)

add_executable(math_test)
target_sources(math_test
    PRIVATE
        test_unit.cpp
)
target_link_libraries(math_test
    PRIVATE
        VectorStore_lib
        GTest::Main
)